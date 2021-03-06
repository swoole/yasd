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

#ifdef HAVE_CONFIG_H
#include "./config.h"
#endif

#include <string>
#include <map>

#include <iostream>
#include <cstdlib>

#include "main/php.h"
#include "main/SAPI.h"
#include "Zend/zend_extensions.h"
#include "Zend/zend_API.h"
#include "ext/standard/info.h"

#include "./php_yasd.h"
#include "yasd_api.h"
#include "yasd_function_status.h"

#include "include/util.h"
#include "include/context.h"
#include "include/global.h"
#include "include/base.h"
#include "include/redirect_file_to_cin.h"

ZEND_DECLARE_MODULE_GLOBALS(yasd)

// clang-format off
PHP_INI_BEGIN()
STD_PHP_INI_ENTRY("yasd.breakpoints_file", "", PHP_INI_ALL, OnUpdateString,
        breakpoints_file, zend_yasd_globals, yasd_globals)
STD_PHP_INI_ENTRY("yasd.debug_mode", "cmd", PHP_INI_ALL, OnUpdateString,
        debug_mode, zend_yasd_globals, yasd_globals)
STD_PHP_INI_ENTRY("yasd.remote_host", "127.0.0.1", PHP_INI_ALL, OnUpdateStringUnempty,
        remote_host, zend_yasd_globals, yasd_globals)
STD_PHP_INI_ENTRY("yasd.remote_port", "9000", PHP_INI_ALL, OnUpdateLong,
        remote_port, zend_yasd_globals, yasd_globals)
STD_PHP_INI_ENTRY("yasd.depth", "1", PHP_INI_ALL, OnUpdateLong,
        depth, zend_yasd_globals, yasd_globals)
STD_PHP_INI_ENTRY("yasd.log_level", "-1", PHP_INI_ALL, OnUpdateLong,
        log_level, zend_yasd_globals, yasd_globals)
STD_PHP_INI_ENTRY("yasd.max_executed_opline_num", "0", PHP_INI_ALL, OnUpdateLong,
        max_executed_opline_num, zend_yasd_globals, yasd_globals)
STD_PHP_INI_ENTRY("yasd.init_file", "", PHP_INI_ALL, OnUpdateStringUnempty,
        init_file, zend_yasd_globals, yasd_globals)

// compatible with phpstorm
STD_PHP_INI_ENTRY("xdebug.coverage_enable", "1", PHP_INI_ALL, OnUpdateLong,
        coverage_enable, zend_yasd_globals, yasd_globals)
STD_PHP_INI_ENTRY("xdebug.profiler_enable", "1", PHP_INI_ALL, OnUpdateLong,
        profiler_enable, zend_yasd_globals, yasd_globals)
STD_PHP_INI_ENTRY("xdebug.remote_autostart", "1", PHP_INI_ALL, OnUpdateLong,
        remote_autostart, zend_yasd_globals, yasd_globals)
STD_PHP_INI_ENTRY("xdebug.remote_connect_back", "0", PHP_INI_ALL, OnUpdateLong,
        remote_autostart, zend_yasd_globals, yasd_globals)
STD_PHP_INI_ENTRY("xdebug.remote_mode", "req", PHP_INI_ALL, OnUpdateString,
        remote_mode, zend_yasd_globals, yasd_globals)
PHP_INI_END()
// clang-format on

static void php_yasd_init_globals(zend_yasd_globals *yasd_globals) {
	memset(yasd_globals, 0, sizeof(zend_yasd_globals));
}

static void check_other_debugger() {
    // We should not use the E_ERROR level, otherwise php --ini would throw fetal error
    if (zend_hash_str_find_ptr(&module_registry, ZEND_STRL("sdebug"))) {
        php_yasd_fatal_error(E_WARNING, "Please don't use the Sdebug and Yasd extensions at the same time!");
    }

    if (zend_hash_str_find_ptr(&module_registry, ZEND_STRL("xdebug"))) {
        php_yasd_fatal_error(E_WARNING, "Please don't use the Xdebug and Yasd extensions at the same time!");
    }
}

PHP_RINIT_FUNCTION(yasd) {
    // use php -e
    // CG(compiler_options) = CG(compiler_options) | ZEND_COMPILE_EXTENDED_STMT;
    if (!(CG(compiler_options) & ZEND_COMPILE_EXTENDED_INFO)) {
        return SUCCESS;
    }

    check_other_debugger();

    yasd_rinit(module_number);

    register_get_cid_function();

    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(yasd) {
    if (!(CG(compiler_options) & ZEND_COMPILE_EXTENDED_INFO)) {
        return SUCCESS;
    }

    global->debugger->handle_stop();

    yasd_rshutdown(module_number);

    return SUCCESS;
}

PHP_MINIT_FUNCTION(yasd) {
    ZEND_INIT_MODULE_GLOBALS(yasd, php_yasd_init_globals, nullptr);
    REGISTER_INI_ENTRIES();

    yasd_minit(module_number);

    yasd_api_module_init(module_number);
    yasd_function_status_minit(module_number);

    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(yasd) {
    resume_execute_ex();
    return SUCCESS;
}

PHP_MINFO_FUNCTION(yasd) {
    char buf[64];

    php_info_print_table_start();
    php_info_print_table_header(2, "Yasd", "enabled");
    php_info_print_table_row(2, "Author", "codinghuang <codinghuang@qq.com>");
    php_info_print_table_row(2, "Version", PHP_YASD_VERSION);
    snprintf(buf, sizeof(buf), "%s %s", __DATE__, __TIME__);
    php_info_print_table_row(2, "Built", buf);
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}

bool is_hit_line_breakpoint(const char *filename, int lineno) {
    auto map_iter = global->breakpoints->find(filename);

    if (map_iter == global->breakpoints->end()) {
        return false;
    }

    auto set_iter = map_iter->second.find(lineno);

    if (set_iter == map_iter->second.end()) {
        return false;
    }

    return true;
}

bool has_line_condition_breakpoint(int lineno) {
    auto map_iter = global->breakpoint_conditions.find(lineno);

    return map_iter != global->breakpoint_conditions.end();
}

bool is_hit_line_condition_breakpoint(int lineno) {
    zval retval;
    auto map_iter = global->breakpoint_conditions.find(lineno);

    if (map_iter == global->breakpoint_conditions.end()) {
        return false;
    }

    std::string condition = map_iter->second;

    if (!yasd::util::eval(const_cast<char *>(condition.c_str()), &retval, nullptr)) {
        return false;
    }

    return Z_TYPE(retval) == IS_TRUE;
}

void add_executed_opline_num(zend_execute_data *frame) {
    yasd::Context *context = global->get_current_context();
    const zend_op *opline = frame->opline + 1;

    // skip ZEND_NOP, every function definition, the location of the class definition, is a ZEND_NOP
    if (opline->opcode == ZEND_NOP) {
        return;
    }

    yasd::CurrentFunctionStatus *function_status = context->function_status.back();
    function_status->executed_opline_num++;
}

bool is_infinite_loop() {
    yasd::Context *context = global->get_current_context();

    yasd::CurrentFunctionStatus *function_status = context->function_status.back();

    if (YASD_G(max_executed_opline_num) > 0 && function_status->executed_opline_num >= YASD_G(max_executed_opline_num)) {
        function_status->executed_opline_num = 0;
        return true;
    }

    return false;
}

ZEND_DLEXPORT void yasd_statement_call(zend_execute_data *frame) {
    // zend_op_array *op_array = &frame->func->op_array;
    const zend_op *online = EG(current_execute_data)->opline;
    const char *filename;
    int lineno;
    int start_lineno;

    yasd::Context *context = global->get_current_context();
    
    add_executed_opline_num(frame);

    if (!EG(current_execute_data)) {
        return;
    }

    filename = yasd::util::get_executed_filename();
    start_lineno = lineno = online->lineno;

    if (yasd::util::is_hit_watch_point()) {
        return global->debugger->handle_request(filename, lineno);
    }

    if (global->do_step) {
        return global->debugger->handle_request(filename, lineno);
    }

    if (global->do_next || global->do_finish) {
        if (global->next_cid == context->cid && context->level <= context->next_level) {
            return global->debugger->handle_request(filename, lineno);
        }
    }

    // infinite loop detection
    if (is_infinite_loop()) {
        return global->debugger->handle_request(filename, lineno);
    }
    
    // The breakpoint check should be last
    if (!global->do_step && !global->do_next) {
        if (!is_hit_line_breakpoint(filename, lineno)) {
            return;
        }

        if (has_line_condition_breakpoint(lineno) && !is_hit_line_condition_breakpoint(lineno)) {
            return;
        }

        return global->debugger->handle_request(filename, lineno);
    }
}

ZEND_DLEXPORT int yasd_zend_startup(zend_extension *extension) {
    return zend_startup_module(&yasd_module_entry);
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Yasd_Zval_getRefCount, ZEND_RETURN_VALUE, 1, IS_LONG, 1)
ZEND_ARG_INFO(0, zv)
ZEND_END_ARG_INFO()

static PHP_FUNCTION(Yasd_Zval_getRefCount) {
    zval *zv;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL(zv)
    ZEND_PARSE_PARAMETERS_END();

    if (!Z_REFCOUNTED_P(zv) && !(Z_TYPE_P(zv) == IS_ARRAY) && !(Z_TYPE_P(zv) == IS_STRING)) {
        RETURN_NULL();
    }
    RETURN_LONG(GC_REFCOUNT(Z_COUNTED_P(zv)));
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Yasd_getOpcodeByName, ZEND_RETURN_VALUE, 1, IS_LONG, 1)
ZEND_ARG_INFO(0, opcodeName)
ZEND_END_ARG_INFO()

static PHP_FUNCTION(Yasd_getOpcodeByName) {
    zend_string *opcode_name;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STR(opcode_name)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

    for (zend_uchar i = 0; i <= ZEND_VM_LAST_OPCODE; i++) {
        const char *tmp = zend_get_opcode_name(i);
        if (strcasecmp(tmp, ZSTR_VAL(opcode_name)) == 0) {
            RETURN_LONG(i);
        }
    }
    RETURN_LONG(-1);
}

// clang-format off
static const zend_function_entry yasd_functions[] = {
    ZEND_FENTRY(Yasd\\Zval\\getRefCount, PHP_FN(Yasd_Zval_getRefCount), arginfo_Yasd_Zval_getRefCount, 0)
    ZEND_FENTRY(Yasd\\getOpcodeByName, PHP_FN(Yasd_getOpcodeByName), arginfo_Yasd_getOpcodeByName, 0)
    PHP_FE_END /* Must be the last line in yasd_functions[] */
};

// clang-format off
zend_module_entry yasd_module_entry = {
    STANDARD_MODULE_HEADER,
    "yasd",           /* Extension name */
    yasd_functions,   /* zend_function_entry */
    PHP_MINIT(yasd),  /* PHP_MINIT - Module initialization */
    NULL,             /* PHP_MSHUTDOWN - Module shutdown */
    PHP_RINIT(yasd),  /* PHP_RINIT - Request initialization */
    PHP_RSHUTDOWN(yasd),             /* PHP_RSHUTDOWN - Request shutdown */
    PHP_MINFO(yasd),  /* PHP_MINFO - Module info */
    PHP_YASD_VERSION, /* Version */
    STANDARD_MODULE_PROPERTIES
};

zend_extension_version_info extension_version_info = {
    ZEND_EXTENSION_API_NO,
    ZEND_EXTENSION_BUILD_ID
};

zend_extension zend_extension_entry = {
    "Yasd",
    PHP_YASD_VERSION,
    "codinghuang",
    "https://github.com/huanghantao",
    "Our Copyright",
    yasd_zend_startup,
    NULL,
    NULL, /* activate_func_t */
    NULL, /* deactivate_func_t */
    NULL, /* message_handler_func_t */
    NULL, /* op_array_handler_func_t */
    yasd_statement_call, /* statement_handler_func_t */
    NULL, /* fcall_begin_handler_func_t */
    NULL, /* fcall_end_handler_func_t */
    NULL, /* op_array_ctor_func_t */
    NULL,  /* op_array_dtor_func_t */

    STANDARD_ZEND_EXTENSION_PROPERTIES
};

#ifdef COMPILE_DL_YASD
ZEND_GET_MODULE(yasd)
#endif
