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
#include "ext/standard/info.h"
#include "./php_yasd.h"

#include "include/util.h"
#include "include/context.h"
#include "include/global.h"
#include "include/base.h"
#include "include/redirect_file_to_cin.h"

ZEND_DECLARE_MODULE_GLOBALS(yasd)

ZEND_BEGIN_ARG_INFO_EX(arginfo_yasd_redirectStdin, 0, 0, 1)
ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

PHP_FUNCTION(redirectStdin) {
    char *file;
    size_t l_file;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STRING(file, l_file)
    ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

    global->redirector = new yasd::RedirectFileToCin(file);
}

// clang-format off
PHP_INI_BEGIN()
STD_PHP_INI_ENTRY("yasd.breakpoints_file", "", PHP_INI_ALL, OnUpdateString,
        breakpoints_file, zend_yasd_globals, yasd_globals)
STD_PHP_INI_ENTRY("yasd.debug_mode", "cmd", PHP_INI_ALL, OnUpdateString,
        debug_mode, zend_yasd_globals, yasd_globals)
STD_PHP_INI_ENTRY("yasd.remote_host", "127.0.0.1", PHP_INI_ALL, OnUpdateString,
        remote_host, zend_yasd_globals, yasd_globals)
STD_PHP_INI_ENTRY("yasd.remote_port", "9000", PHP_INI_ALL, OnUpdateLong,
        remote_port, zend_yasd_globals, yasd_globals)
STD_PHP_INI_ENTRY("yasd.depth", "1", PHP_INI_ALL, OnUpdateLong,
        depth, zend_yasd_globals, yasd_globals)
STD_PHP_INI_ENTRY("yasd.log_level", "-1", PHP_INI_ALL, OnUpdateLong,
        log_level, zend_yasd_globals, yasd_globals)
PHP_INI_END()
// clang-format on

static void php_yasd_init_globals(zend_yasd_globals *yasd_globals) {}

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

    // global->entry_file = SG(request_info).path_translated;

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

ZEND_DLEXPORT void yasd_statement_call(zend_execute_data *frame) {
    if (UNEXPECTED(global->is_detach)) {
        return;
    }

    // zend_op_array *op_array = &frame->func->op_array;
    const zend_op *online = EG(current_execute_data)->opline;
    const char *filename;
    int lineno;
    int start_lineno;

    yasd::Context *context = global->get_current_context();

    if (!EG(current_execute_data)) {
        return;
    }

    filename = yasd::Util::get_executed_filename();
    start_lineno = lineno = online->lineno;

    if (yasd::Util::is_hit_watch_point()) {
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

    if (!global->do_step && !global->do_next) {
        auto map_iter = global->breakpoints->find(filename);

        if (map_iter == global->breakpoints->end()) {
            return;
        }

        auto set_iter = map_iter->second.find(lineno);

        if (set_iter == map_iter->second.end()) {
            return;
        }

        return global->debugger->handle_request(filename, lineno);
    }
}

ZEND_DLEXPORT int yasd_zend_startup(zend_extension *extension) {
    return zend_startup_module(&yasd_module_entry);
}

// clang-format off
static const zend_function_entry yasd_functions[] = {
    PHP_FE(redirectStdin, arginfo_yasd_redirectStdin)
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
