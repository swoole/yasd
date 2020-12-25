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

#include "include/util.h"
#include "include/context.h"
#include "include/global.h"
#include "include/base.h"

#include "main/SAPI.h"

extern sapi_module_struct sapi_module;

static void (*old_execute_ex)(zend_execute_data *execute_data);

bool skip_swoole_library(zend_execute_data *execute_data) {
    zend_op_array *op_array = &(execute_data->func->op_array);

    if (op_array && op_array->filename &&
        strncmp("@swoole-src/library", ZSTR_VAL(op_array->filename), sizeof("@swoole-src/library") - 1) == 0) {
        old_execute_ex(execute_data);
        return true;
    }
    return false;
}

bool skip_eval(zend_execute_data *execute_data) {
    zend_op_array *op_array = &(execute_data->func->op_array);

    if (op_array && op_array->filename &&
        strncmp("xdebug://debug-eval", ZSTR_VAL(op_array->filename), sizeof("xdebug://debug-eval") - 1) == 0) {
        old_execute_ex(execute_data);
        return true;
    }
    return false;
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

void yasd_execute_ex(zend_execute_data *execute_data) {
    if (skip_swoole_library(execute_data) || skip_eval(execute_data)) {
        return;
    }

    if (!global) {
        old_execute_ex(execute_data);
        return;
    }

    yasd::Context *context = global->get_current_context();

    context->level++;
    yasd::StackFrame *frame = save_prev_stack_frame(execute_data);
    old_execute_ex(execute_data);
    // reduce the function call trace
    drop_prev_stack_frame(frame);
    // reduce the level of function call
    context->level--;
    // clear watch point
    clear_watch_point(execute_data);
}

void register_get_cid_function() {
    if (zend_hash_str_find_ptr(&module_registry, ZEND_STRL("swoole")) &&
        (strcmp("cli", sapi_module.name) == 0 || strcmp("phpdbg", sapi_module.name) == 0)) {
        zend_string *classname = zend_string_init(ZEND_STRL("Swoole\\Coroutine"), 0);
        zend_class_entry *class_handle = zend_lookup_class(classname);
        zend_string_release(classname);

        get_cid_function = reinterpret_cast<zend_function *>(
            zend_hash_str_find_ptr(&(class_handle->function_table), ZEND_STRL("getcid")));
    }
}

void yasd_rinit(int module_number) {
    global = new yasd::Global();
}

void yasd_minit(int module_number) {
    replace_execute_ex();
}

void replace_execute_ex() {
    old_execute_ex = zend_execute_ex;
    zend_execute_ex = yasd_execute_ex;
}

void resume_execute_ex() {
    zend_execute_ex = old_execute_ex;
    old_execute_ex = nullptr;
}
