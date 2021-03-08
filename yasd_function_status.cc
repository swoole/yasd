#include "./yasd_function_status.h"

zend_class_entry *yasd_function_status_ce;
zend_object_handlers yasd_function_status_handlers;

void yasd_function_status_minit(int module_number)
{
    YASD_INIT_CLASS_ENTRY_BASE(yasd_function_status, "Yasd\\FunctionStatus", nullptr, nullptr);

    zend_declare_property_string(yasd_function_status_ce, ZEND_STRL("functionName"), "", ZEND_ACC_PUBLIC);
    zend_declare_property_long(yasd_function_status_ce, ZEND_STRL("executeTime"), 0, ZEND_ACC_PUBLIC);
    zend_declare_property_long(yasd_function_status_ce, ZEND_STRL("level"), 0, ZEND_ACC_PUBLIC);
    zend_declare_property_string(yasd_function_status_ce, ZEND_STRL("parentFunctionName"), "", ZEND_ACC_PUBLIC);
}
