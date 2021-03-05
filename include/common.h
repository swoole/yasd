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

#pragma once

#include "main/php.h"

#ifdef __cplusplus
#define YASD_EXTERN_C_BEGIN extern "C" {
#define YASD_EXTERN_C_END }
#else
#define YASD_EXTERN_C_BEGIN
#define YASD_EXTERN_C_END
#endif

#define YASD_HASHTABLE_FOREACH_START(ht, _val)                                                                         \
    ZEND_HASH_FOREACH_VAL(ht, _val);                                                                                   \
    {
#define YASD_HASHTABLE_FOREACH_START2(ht, k, klen, ktype, _val)                                                        \
    zend_string *_foreach_key;                                                                                         \
    ZEND_HASH_FOREACH_STR_KEY_VAL(ht, _foreach_key, _val);                                                             \
    if (!_foreach_key) {                                                                                               \
        k = NULL;                                                                                                      \
        klen = 0;                                                                                                      \
        ktype = 0;                                                                                                     \
    } else {                                                                                                           \
        k = ZSTR_VAL(_foreach_key), klen = ZSTR_LEN(_foreach_key);                                                     \
        ktype = 1;                                                                                                     \
    }                                                                                                                  \
    {
#define YASD_HASHTABLE_FOREACH_END()                                                                                   \
    }                                                                                                                  \
    ZEND_HASH_FOREACH_END();

/* PHP 7.3 compatibility macro {{{*/
#ifndef GC_ADDREF
#define GC_ADDREF(ref) ++GC_REFCOUNT(ref)
#define GC_DELREF(ref) --GC_REFCOUNT(ref)
#endif

#ifndef ZEND_CLOSURE_OBJECT
#define ZEND_CLOSURE_OBJECT(func) (zend_object *) func->op_array.prototype
#endif

/* PHP 7.4 compatibility macro {{{*/
#ifndef ZEND_COMPILE_EXTENDED_STMT
#define ZEND_COMPILE_EXTENDED_STMT ZEND_COMPILE_EXTENDED_INFO
#endif

#ifndef ZVAL_EMPTY_ARRAY
#define ZVAL_EMPTY_ARRAY(zval) (array_init((zval)))
#endif
#ifndef RETVAL_EMPTY_ARRAY
#define RETVAL_EMPTY_ARRAY() ZVAL_EMPTY_ARRAY(return_value)
#endif
#ifndef RETURN_EMPTY_ARRAY
#define RETURN_EMPTY_ARRAY()                                                                                           \
    do {                                                                                                               \
        RETVAL_EMPTY_ARRAY();                                                                                          \
        return;                                                                                                        \
    } while (0)
#endif

#ifndef ZEND_THIS
#define ZEND_THIS (&EX(This))
#endif

#ifndef ZEND_THIS_OBJECT
#define ZEND_THIS_OBJECT Z_OBJ_P(ZEND_THIS)
#endif

#ifndef E_FATAL_ERRORS
#define E_FATAL_ERRORS (E_ERROR | E_CORE_ERROR | E_COMPILE_ERROR | E_USER_ERROR | E_RECOVERABLE_ERROR | E_PARSE)
#endif
/*}}}*/

/* PHP 8 compatibility macro {{{*/
#if PHP_VERSION_ID < 80000
#define yasd_zend7_object zval
#define YASD_Z7_OBJ_P(object) Z_OBJ_P(object)
#define YASD_Z8_OBJ_P(zobj) zobj
#else
#define yasd_zend7_object zend_object
#define YASD_Z7_OBJ_P(object) object
#define YASD_Z8_OBJ_P(zobj) Z_OBJ_P(zobj)
#endif
/*}}}*/

//----------------------------------Class API------------------------------------

#define YASD_Z_OBJCE_NAME_VAL_P(zobject) ZSTR_VAL(Z_OBJCE_P(zobject)->name)

/* PHP 7 class declaration macros */

#define YASD_INIT_CLASS_ENTRY_BASE(module, namespace_name, methods, parent_ce)                   \
    do {                                                                                                               \
        zend_class_entry _##module##_ce = {};                                                                          \
        INIT_CLASS_ENTRY(_##module##_ce, namespace_name, methods);                                                     \
        module##_ce = zend_register_internal_class_ex(&_##module##_ce, parent_ce);                                     \
    } while (0)

#define YASD_INIT_CLASS_ENTRY(module, namespace_name, snake_name, short_name, methods)                                   \
    YASD_INIT_CLASS_ENTRY_BASE(module, namespace_name, snake_name, short_name, methods, NULL);                           \
    memcpy(&module##_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers))

#define YASD_INIT_CLASS_ENTRY_EX(module, namespace_name, snake_name, short_name, methods, parent_module)                 \
    YASD_INIT_CLASS_ENTRY_BASE(module, namespace_name, snake_name, short_name, methods, parent_module##_ce);             \
    memcpy(&module##_handlers, &parent_module##_handlers, sizeof(zend_object_handlers))

#define YASD_INIT_CLASS_ENTRY_EX2(                                                                                       \
    module, namespace_name, snake_name, short_name, methods, parent_module_ce, parent_module_handlers)                 \
    YASD_INIT_CLASS_ENTRY_BASE(module, namespace_name, snake_name, short_name, methods, parent_module_ce);               \
    memcpy(&module##_handlers, parent_module_handlers, sizeof(zend_object_handlers))

// Data Object: no methods, no parent
#define YASD_INIT_CLASS_ENTRY_DATA_OBJECT(module, namespace_name)                                                        \
    YASD_INIT_CLASS_ENTRY_BASE(module, namespace_name, NULL, NULL, NULL, NULL);                                          \
    memcpy(&module##_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers))

# if PHP_VERSION_ID >= 70300
#  define YASD_ZEND_CONSTANT_MODULE_NUMBER(v) ZEND_CONSTANT_MODULE_NUMBER((v))
# else
#  define YASD_ZEND_CONSTANT_MODULE_NUMBER(v) ((v)->module_number)
# endif

zend_bool yasd_zend_hash_is_recursive(zend_array* ht);
zend_bool yasd_zend_hash_apply_protection_begin(zend_array* ht);
zend_bool yasd_zend_hash_apply_protection_end(zend_array* ht);

static void yasd_zend_update_property_null_ex(zend_class_entry *scope, zval *object, zend_string *s) {
    zval tmp;

    ZVAL_NULL(&tmp);
    zend_update_property_ex(scope, YASD_Z8_OBJ_P(object), s, &tmp);
}

static zval *yasd_zend_read_property(zend_class_entry *ce, zval *obj, const char *s, int len, int silent) {
    zval rv, *property = zend_read_property(ce, YASD_Z8_OBJ_P(obj), s, len, silent, &rv);
    if (UNEXPECTED(property == &EG(uninitialized_zval))) {
        return nullptr;
    }
    return property;
}

static zval *yasd_zend_read_property_ex(zend_class_entry *ce, zval *obj, zend_string *s, int silent) {
    zval rv, *property = zend_read_property_ex(ce, YASD_Z8_OBJ_P(obj), s, silent, &rv);
    if (UNEXPECTED(property == &EG(uninitialized_zval))) {
        return nullptr;
    }
    return property;
}

namespace yasd {
enum Color {
    YASD_ECHO_RED,
    YASD_ECHO_GREEN,
    YASD_ECHO_YELLOW,
    YASD_ECHO_BLUE,
    YASD_ECHO_MAGENTA,
    YASD_ECHO_CYAN,
    YASD_ECHO_WHITE,
};

}  // namespace yasd

// #define YASD_ECHO_RED "\e[31m%s\e[0m"
// #define YASD_ECHO_GREEN "\e[32m%s\e[0m"
// #define YASD_ECHO_YELLOW "\e[33m%s\e[0m"
// #define YASD_ECHO_BLUE "\e[34m%s\e[0m"
// #define YASD_ECHO_MAGENTA "\e[35m%s\e[0m"
// #define YASD_ECHO_CYAN "\e[36m%s\e[0m"
// #define YASD_ECHO_WHITE "\e[37m%s\e[0m"

#define YASD_MSG_SIZE 16384

extern char yasd_info_buf[YASD_MSG_SIZE];
