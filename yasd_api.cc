#include "main/php.h"
#include "main/SAPI.h"
#include "Zend/zend_extensions.h"
#include "Zend/zend_API.h"
#include "Zend/zend_exceptions.h"
#include "ext/standard/info.h"
#include "./php_yasd.h"

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

static const zend_function_entry yasd_api_functions[] = {
    ZEND_FENTRY(Yasd\\Api\\setBreakpoint, PHP_FN(Yasd_Api_setBreakpoint), arginfo_Yasd_Api_setBreakpoint, 0)
    PHP_FE_END /* Must be the last line in yasd_functions[] */
};

int yasd_api_module_init(int module_number)
{
    if (zend_register_functions(NULL, yasd_api_functions, NULL, MODULE_PERSISTENT) != SUCCESS) {
        return FAILURE;
    }

    return SUCCESS;
}
