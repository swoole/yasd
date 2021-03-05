/*
  +----------------------------------------------------------------------+
  | Yasd                                                                 |
  +----------------------------------------------------------------------+
  | This source file is subject to version 2.0 of the Apache license,    |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.apache.org/licenses/LICENSE-2.0.html                      |
  | If you did not receive a copy of the Apache2.0 license and are unable|
  | to obtain it through the world-wide-web, please send a note to       |
  | license@swoole.com so we can mail you a copy immediately.            |
  +----------------------------------------------------------------------+
  | Author: codinghuang  <codinghuang@qq.com>                            |
  +----------------------------------------------------------------------+
*/

#include <iostream>
#include <boost/filesystem.hpp>

#include "include/util.h"
#include "include/context.h"
#include "include/global.h"
#include "include/base.h"
#include "php_yasd_cxx.h"
#include "yasd_function_status.h"

#include "main/SAPI.h"
#include "Zend/zend_exceptions.h"

extern sapi_module_struct sapi_module;

static void (*old_execute_ex)(zend_execute_data *execute_data);

void execute_init_file() {
    zval retval;
    zend_file_handle file_handle;
    zend_op_array *op_array;

    if (!YASD_G(init_file)) {
        return;
    }

    if(access(YASD_G(init_file), F_OK) != 0 ) {
        yasd::Util::printfln_info(yasd::Color::YASD_ECHO_RED, "[yasd] init_file is configured, but does not exist");
        exit(255);
    }

    memset(&file_handle, 0, sizeof(zend_file_handle));
    file_handle.type = ZEND_HANDLE_FILENAME;
    file_handle.filename = YASD_G(init_file);

    CG(compiler_options) &= ~ZEND_COMPILE_EXTENDED_INFO;
    op_array = zend_compile_file(&file_handle, ZEND_EVAL);

    zend_execute_ex = old_execute_ex;
    zend_execute(op_array, &retval);
    zend_execute_ex = yasd_execute_ex;

    // we need to set ZEND_COMPILE_EXTENDED_INFO after the zend_execute function,
    // not after zend_compile_file, because zend_execute may include files
    CG(compiler_options) |= ZEND_COMPILE_EXTENDED_INFO;

    zend_destroy_file_handle(&file_handle);
    destroy_op_array(op_array);
    efree_size(op_array, sizeof(zend_op_array));
}

bool skip_swoole_library_eval(zend_execute_data *execute_data) {
    zend_op_array *op_array = &(execute_data->func->op_array);

    if (op_array && op_array->filename &&
        strncmp("@swoole-src/library", ZSTR_VAL(op_array->filename), sizeof("@swoole-src/library") - 1) == 0) {
        old_execute_ex(execute_data);
        return true;
    }
    return false;
}

bool skip_yasd_eval(zend_execute_data *execute_data) {
    zend_op_array *op_array = &(execute_data->func->op_array);

    if (op_array && op_array->filename &&
        strncmp("yasd://debug-eval", ZSTR_VAL(op_array->filename), sizeof("yasd://debug-eval") - 1) == 0) {
        old_execute_ex(execute_data);
        return true;
    }
    return false;
}

bool skip_protobuf_eval(zend_execute_data *execute_data) {
    zend_op_array *op_array = &(execute_data->func->op_array);

    if (op_array && op_array->filename &&
        strncmp("autoload_register.php", ZSTR_VAL(op_array->filename), sizeof("autoload_register.php") - 1) == 0) {
        old_execute_ex(execute_data);
        return true;
    }
    return false;
}

bool skip_eval(zend_execute_data *execute_data) {
    if (!skip_swoole_library_eval(execute_data) && !skip_yasd_eval(execute_data) && !skip_protobuf_eval(execute_data)) {
        return false;
    }
    return true;
}

yasd::StackFrame *save_prev_stack_frame(zend_execute_data *execute_data) {
    yasd::Context *context = global->get_current_context();

    if (!EG(current_execute_data)->prev_execute_data) {
        return nullptr;
    }

    yasd::StackFrame *frame = new yasd::StackFrame();
    frame->level = context->level;
    frame->filename = yasd::Util::get_prev_executed_filename();
    frame->lineno = yasd::Util::get_prev_executed_file_lineno();
    frame->function_name = yasd::Util::get_prev_executed_function_name();
    context->strace->emplace_back(frame);
    return frame;
}

void drop_prev_stack_frame(yasd::StackFrame *frame) {
    yasd::Context *context = global->get_current_context();

    context->strace->pop_back();
    delete frame;
}

void clear_watch_point(zend_execute_data *execute_data) {
    zend_function *func = execute_data->func;

    auto var_watchpoint = global->watchPoints.var_watchpoint.find(func);

    if (var_watchpoint == global->watchPoints.var_watchpoint.end()) {
        return;
    }

    var_watchpoint->second->begin();

    auto iter = var_watchpoint->second->begin();
    while (iter != var_watchpoint->second->end()) {
        var_watchpoint->second->erase(iter++);
    }
}

yasd::CurrentFunctionStatus *save_current_function_status(zend_execute_data *execute_data) {
    yasd::Context *context = global->get_current_context();

    yasd::CurrentFunctionStatus *current_function_status = new yasd::CurrentFunctionStatus();
    current_function_status->start_time = yasd::Util::microtime();
    current_function_status->execute_data = execute_data;
    context->function_status.emplace_back(current_function_status);
    return current_function_status;
}

void analyze_function(yasd::CurrentFunctionStatus *function_status) {
    zval argv[1];
    zval *object = &argv[0];
    long execute_time;
    zend_string *function_name = nullptr;
    zend_string *parent_function_name = nullptr;

    function_name = function_status->execute_data->func->common.function_name;
    function_status->end_time = yasd::Util::microtime();
    execute_time = function_status->end_time - function_status->start_time;

    if (!global->onGreaterThanMilliseconds) {
        return;
    }

    if (execute_time < YASD_G(max_executed_milliseconds)) {
        return;
    }

    object_init_ex(object, yasd_function_status_ce);

    zend_update_property_long(
            yasd_function_status_ce, YASD_Z8_OBJ_P(object), ZEND_STRL("executeTime"), execute_time);
    zend_update_property_str(
            yasd_function_status_ce, YASD_Z8_OBJ_P(object), ZEND_STRL("functionName"), 
            function_name ? function_name : zend_empty_string);

    if (function_status->execute_data->prev_execute_data && function_status->execute_data->prev_execute_data->func) {
        parent_function_name = function_status->execute_data->prev_execute_data->func->common.function_name;
        zend_update_property_str(
            yasd_function_status_ce, YASD_Z8_OBJ_P(object), ZEND_STRL("parentFunctionName"), 
            parent_function_name ? parent_function_name : zend_empty_string);
    } else {
        zend_update_property_null(yasd_function_status_ce, YASD_Z8_OBJ_P(object), ZEND_STRL("parentFunctionName"));
    }

    zend_execute_ex = old_execute_ex;
    zend::function::call(global->onGreaterThanMilliseconds, 1, argv, nullptr);
    zend_execute_ex = yasd_execute_ex;

    zval_ptr_dtor(&argv[0]);
}

void drop_current_function_status(yasd::CurrentFunctionStatus *function_status) {
    yasd::Context *context = global->get_current_context();

    context->function_status.pop_back();
    delete function_status;
}

void yasd_execute_ex(zend_execute_data *execute_data) {
    // if not set -e, we will not initialize global
    if (!(CG(compiler_options) & ZEND_COMPILE_EXTENDED_INFO)) {
        old_execute_ex(execute_data);
        return;
    }

    if (skip_eval(execute_data)) {
        return;
    }

    if (UNEXPECTED(global->first_entry)) {
        ZVAL_ARR(&global->globals, &EG(symbol_table));
        global->debugger->init();
        global->first_entry = false;
    }

    yasd::Context *context = global->get_current_context();

    context->level++;
    yasd::StackFrame *frame = save_prev_stack_frame(execute_data);
    yasd::CurrentFunctionStatus *function_status = save_current_function_status(execute_data);
    old_execute_ex(execute_data);
    analyze_function(function_status);
    drop_current_function_status(function_status);
    // reduce the function call trace
    drop_prev_stack_frame(frame);
    // reduce the level of function call
    context->level--;
    // clear watch point
    clear_watch_point(execute_data);
}

void register_get_cid_function() {
    // if not install swoole or not cli, we will not register get_cid_function. so get_cid will always return 0
    if (zend_hash_str_find_ptr(&module_registry, ZEND_STRL("swoole")) && (strcmp("cli", sapi_module.name) == 0)) {
        zend_string *classname = zend_string_init(ZEND_STRL("Swoole\\Coroutine"), 0);
        zend_class_entry *class_handle = zend_lookup_class(classname);
        zend_string_release(classname);

        get_cid_function = reinterpret_cast<zend_function *>(
            zend_hash_str_find_ptr(&(class_handle->function_table), ZEND_STRL("getcid")));
    }
}

void disable_opcache_optimizer() {
    zend_string *key = zend_string_init(ZEND_STRL("opcache.optimization_level"), 1);
    zend_string *value = zend_string_init(ZEND_STRL("0"), 1);

    zend_alter_ini_entry(key, value, ZEND_INI_SYSTEM, ZEND_INI_STAGE_STARTUP);

    zend_string_release(key);
    zend_string_release(value);
}

void yasd_rinit(int module_number) {
    global = new yasd::Global();

    execute_init_file();

    disable_opcache_optimizer();
}

void yasd_rshutdown(int module_number) {
    delete global;
    global = nullptr;
}

void yasd_minit(int module_number) {
    replace_execute_ex();

    // it seems that -e does not work in PHP-FPM mode. so we need to add ZEND_COMPILE_EXTENDED_INFO ourselves.
    if (strcmp("cgi-fcgi", sapi_module.name) == 0 || strcmp("fpm-fcgi", sapi_module.name) == 0) {
        CG(compiler_options) |= ZEND_COMPILE_EXTENDED_INFO;
    }
}

void replace_execute_ex() {
    old_execute_ex = zend_execute_ex;
    zend_execute_ex = yasd_execute_ex;
}

void resume_execute_ex() {
    zend_execute_ex = old_execute_ex;
    old_execute_ex = nullptr;
}
