#include "main/php.h"
#include "main/SAPI.h"
#include "Zend/zend_extensions.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "ext/standard/info.h"
#include "./php_yasd_cxx.h"

#include "include/global.h"

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Yasd_Api_setBreakpoint, ZEND_RETURN_VALUE, 2, _IS_BOOL, 0)
ZEND_ARG_INFO(0, fileAbsolutePath)
ZEND_ARG_INFO(0, lineno)
ZEND_END_ARG_INFO()

static PHP_FUNCTION(Yasd_Api_setBreakpoint) {
    zend_string *file_absolute_path;
    zend_long lineno;

    ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_STR(file_absolute_path)
    Z_PARAM_LONG(lineno)
    ZEND_PARSE_PARAMETERS_END();

    if (!global) {
        zend_throw_exception(zend_ce_exception, "debugger is not init", 0);
        RETURN_FALSE;
    }

    auto iter = global->breakpoints->find(ZSTR_VAL(file_absolute_path));

    if (iter != global->breakpoints->end()) {
        iter->second.insert(lineno);
    } else {
        std::set<int> lineno_set;
        lineno_set.insert(lineno);
        global->breakpoints->insert(std::make_pair(ZSTR_VAL(file_absolute_path), lineno_set));
    }

    RETURN_TRUE;
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Yasd_Api_setMaxExecutedOplineNum, ZEND_RETURN_VALUE, 1, _IS_BOOL, 0)
ZEND_ARG_INFO(0, num)
ZEND_END_ARG_INFO()

static PHP_FUNCTION(Yasd_Api_setMaxExecutedOplineNum) {
    zend_long num;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_LONG(num)
    ZEND_PARSE_PARAMETERS_END();

    if (!global) {
        zend_throw_exception(zend_ce_exception, "debugger is not init", 0);
        RETURN_FALSE;
    }

    YASD_G(max_executed_opline_num) = num;

    RETURN_TRUE;
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Yasd_Api_onGreaterThanMilliseconds, ZEND_RETURN_VALUE, 2, _IS_BOOL, 0)
ZEND_ARG_INFO(0, milliseconds)
ZEND_ARG_INFO(0, callback)
ZEND_END_ARG_INFO()

static PHP_FUNCTION(Yasd_Api_onGreaterThanMilliseconds) {
    zend_long milliseconds;
    zval *zcallback = nullptr;
    char *func_name;

    ZEND_PARSE_PARAMETERS_START(2, 2)
    Z_PARAM_LONG(milliseconds)
    Z_PARAM_ZVAL_EX(zcallback, 1, 0)
    ZEND_PARSE_PARAMETERS_END();

    if (!global) {
        zend_throw_exception(zend_ce_exception, "debugger is not init", 0);
        RETURN_FALSE;
    }

    YASD_G(max_executed_milliseconds) = milliseconds;

    if (global->onGreaterThanMilliseconds) {
        efree(global->onGreaterThanMilliseconds);
        global->onGreaterThanMilliseconds = nullptr;
    }

    global->onGreaterThanMilliseconds = (zend_fcall_info_cache *) ecalloc(1, sizeof(zend_fcall_info_cache));
    if (!yasd_zend_is_callable_ex(zcallback, nullptr, 0, &func_name, 0, global->onGreaterThanMilliseconds, nullptr)) {
        php_yasd_fatal_error(E_WARNING, "function '%s' is not callable", func_name);
        efree(func_name);
        efree(global->onGreaterThanMilliseconds);
        RETURN_FALSE;
    }
    efree(func_name);
    yasd_zend_fci_cache_persist(global->onGreaterThanMilliseconds);

    RETURN_TRUE;
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Yasd_Api_setRemoteHost, ZEND_RETURN_VALUE, 1, _IS_BOOL, 0)
ZEND_ARG_INFO(0, host)
ZEND_END_ARG_INFO()

static PHP_FUNCTION(Yasd_Api_setRemoteHost) {
    zend_string *host;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_STR(host)
    ZEND_PARSE_PARAMETERS_END();

    if (!global) {
        zend_throw_exception(zend_ce_exception, "debugger is not init", 0);
        RETURN_FALSE;
    }

    YASD_G(remote_host) = strdup(ZSTR_VAL(host));

    RETURN_TRUE;
}

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_Yasd_Api_onEnterFunction, ZEND_RETURN_VALUE, 1, _IS_BOOL, 0)
ZEND_ARG_INFO(0, callback)
ZEND_END_ARG_INFO()

static PHP_FUNCTION(Yasd_Api_onEnterFunction) {
    zval *zcallback = nullptr;
    char *func_name;

    ZEND_PARSE_PARAMETERS_START(1, 1)
    Z_PARAM_ZVAL_EX(zcallback, 1, 0)
    ZEND_PARSE_PARAMETERS_END();

    if (!global) {
        zend_throw_exception(zend_ce_exception, "debugger is not init", 0);
        RETURN_FALSE;
    }

    if (global->onEnterFunction) {
        efree(global->onEnterFunction);
        global->onEnterFunction = nullptr;
    }

    global->onEnterFunction = (zend_fcall_info_cache *) ecalloc(1, sizeof(zend_fcall_info_cache));
    if (!yasd_zend_is_callable_ex(zcallback, nullptr, 0, &func_name, 0, global->onEnterFunction, nullptr)) {
        php_yasd_fatal_error(E_WARNING, "function '%s' is not callable", func_name);
        efree(func_name);
        efree(global->onEnterFunction);
        RETURN_FALSE;
    }
    efree(func_name);
    yasd_zend_fci_cache_persist(global->onEnterFunction);

    RETURN_TRUE;
}

static const zend_function_entry yasd_api_functions[] = {
    ZEND_FENTRY(Yasd\\Api\\setBreakpoint, PHP_FN(Yasd_Api_setBreakpoint), arginfo_Yasd_Api_setBreakpoint, 0)
    ZEND_FENTRY(Yasd\\Api\\setMaxExecutedOplineNum, PHP_FN(Yasd_Api_setMaxExecutedOplineNum), arginfo_Yasd_Api_setMaxExecutedOplineNum, 0)
    ZEND_FENTRY(Yasd\\Api\\setRemoteHost, PHP_FN(Yasd_Api_setRemoteHost), arginfo_Yasd_Api_setRemoteHost, 0)
    ZEND_FENTRY(Yasd\\Api\\onGreaterThanMilliseconds, PHP_FN(Yasd_Api_onGreaterThanMilliseconds), arginfo_Yasd_Api_onGreaterThanMilliseconds, 0)
    ZEND_FENTRY(Yasd\\Api\\onEnterFunction, PHP_FN(Yasd_Api_onEnterFunction), arginfo_Yasd_Api_onEnterFunction, 0)
    PHP_FE_END /* Must be the last line in yasd_api_functions[] */
};

int yasd_api_module_init(int module_number)
{
    if (zend_register_functions(NULL, yasd_api_functions, NULL, MODULE_PERSISTENT) != SUCCESS) {
        return FAILURE;
    }

    return SUCCESS;
}
