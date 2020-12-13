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

zend_bool yasd_zend_hash_apply_protection_begin(zend_array* ht) {
    if (!ht) {
        return 1;
    }
    if (ZEND_HASH_GET_APPLY_COUNT(ht) > 0) {
        return 0;
    }
    if (ZEND_HASH_APPLY_PROTECTION(ht)) {
        ZEND_HASH_INC_APPLY_COUNT(ht);
    }
    return 1;
}

zend_bool yasd_zend_hash_apply_protection_end(zend_array* ht) {
    if (!ht) {
        return 1;
    }
    if (ZEND_HASH_GET_APPLY_COUNT(ht) == 0) {
        return 0;
    }
    if (ZEND_HASH_APPLY_PROTECTION(ht)) {
        ZEND_HASH_DEC_APPLY_COUNT(ht);
    }
    return 1;
}
#else /* PHP 7.3 or later */
zend_bool yasd_zend_hash_is_recursive(zend_array* ht) {
    return GC_IS_RECURSIVE(ht);
}

zend_bool yasd_zend_hash_apply_protection_begin(zend_array* ht) {
if (GC_IS_RECURSIVE(ht)) {
        return 0;
    }
    if (!(GC_FLAGS(ht) & GC_IMMUTABLE)) {
        GC_PROTECT_RECURSION(ht);
    }
    return 1;
}

zend_bool yasd_zend_hash_apply_protection_end(zend_array* ht) {
    if (!GC_IS_RECURSIVE(ht)) {
        return 0;
    }
    if (!(GC_FLAGS(ht) & GC_IMMUTABLE)) {
        GC_UNPROTECT_RECURSION(ht);
    }
    return 1;
}
#endif
