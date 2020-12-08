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

#define BEGIN_EXTERN_C() extern "C" {
#define END_EXTERN_C() }

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

#ifndef ZEND_THIS
#define ZEND_THIS (&EX(This))
#endif

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

# if PHP_VERSION_ID >= 70300
#  define YASD_ZEND_CONSTANT_MODULE_NUMBER(v) ZEND_CONSTANT_MODULE_NUMBER((v))
# else
#  define YASD_ZEND_CONSTANT_MODULE_NUMBER(v) ((v)->module_number)
# endif

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