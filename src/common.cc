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
#include "include/common.h"

char yasd_info_buf[YASD_MSG_SIZE];

#ifdef ZEND_HASH_GET_APPLY_COUNT /* PHP 7.2 or earlier recursion protection */
zend_bool yasd_zend_hash_is_recursive(zend_array* ht) {
    return (ZEND_HASH_GET_APPLY_COUNT(ht) > 0);
}
#else /* PHP 7.3 or later */
zend_bool yasd_zend_hash_is_recursive(zend_array* ht) {
    return GC_IS_RECURSIVE(ht);
}
#endif
