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

#define BEGIN_EXTERN_C() extern "C" {
#define END_EXTERN_C() }

#define YASD_HASHTABLE_FOREACH_START(ht, _val) ZEND_HASH_FOREACH_VAL(ht, _val);  {
#define YASD_HASHTABLE_FOREACH_START2(ht, k, klen, ktype, _val) zend_string *_foreach_key;\
    ZEND_HASH_FOREACH_STR_KEY_VAL(ht, _foreach_key, _val); \
    if (!_foreach_key) {k = NULL; klen = 0; ktype = 0;} \
    else {k = ZSTR_VAL(_foreach_key), klen=ZSTR_LEN(_foreach_key); ktype = 1;} {
#define YASD_HASHTABLE_FOREACH_END()                 } ZEND_HASH_FOREACH_END();

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