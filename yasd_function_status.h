#pragma once

#include "php_yasd_cxx.h"

extern zend_class_entry *yasd_function_status_ce;
extern zend_object_handlers yasd_function_status_handlers;

void yasd_function_status_minit(int module_number);
