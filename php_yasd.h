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

#ifndef PHP_YASD_H_
#define PHP_YASD_H_

#include "main/php.h"
#include "Zend/zend_types.h"
#include "zend_closures.h"
#include "zend_exceptions.h"
#include "include/common.h"

extern zend_module_entry yasd_module_entry;
#define phpext_yasd_ptr &yasd_module_entry

#define PHP_YASD_VERSION "0.3.8"

ZEND_BEGIN_MODULE_GLOBALS(yasd)
    char *breakpoints_file;
    char *init_file;
    char *debug_mode;
    char *remote_host;
    uint16_t remote_port;
    uint16_t depth;
    int log_level;
    ssize_t max_executed_opline_num;
    ssize_t max_executed_milliseconds;

    // compatible with phpstorm
    int coverage_enable;
    int profiler_enable;
    int remote_autostart;
    int remote_connect_back;
    char *remote_mode;
ZEND_END_MODULE_GLOBALS(yasd)

extern ZEND_DECLARE_MODULE_GLOBALS(yasd);

#define YASD_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(yasd, v)

#define php_yasd_fatal_error(level, fmt_str, ...) \
        php_error_docref(NULL, level, (const char *) (fmt_str), ##__VA_ARGS__)

static inline zend_bool yasd_zend_is_callable_at_frame(zval *zcallable,
                                                        zval *zobject,
                                                        zend_execute_data *frame,
                                                        uint check_flags,
                                                        char **callable_name,
                                                        size_t *callable_name_len,
                                                        zend_fcall_info_cache *fci_cache,
                                                        char **error) {
    zend_string *name;
    zend_bool ret;
#if PHP_VERSION_ID < 80000
    ret = zend_is_callable_ex(zcallable, zobject ? Z_OBJ_P(zobject) : NULL, check_flags, &name, fci_cache, error);
#else
    ret = zend_is_callable_at_frame(zcallable, zobject ? Z_OBJ_P(zobject) : NULL, frame, check_flags, fci_cache, error);
    name = zend_get_callable_name_ex(zcallable, zobject ? Z_OBJ_P(zobject) : NULL);
#endif
    if (callable_name) {
        *callable_name = estrndup(ZSTR_VAL(name), ZSTR_LEN(name));
    }
    if (callable_name_len) {
        *callable_name_len = ZSTR_LEN(name);
    }
    zend_string_release(name);
    return ret;
}

static inline zend_bool yasd_zend_is_callable_ex(zval *zcallable,
                                                  zval *zobject,
                                                  uint check_flags,
                                                  char **callable_name,
                                                  size_t *callable_name_len,
                                                  zend_fcall_info_cache *fci_cache,
                                                  char **error) {
    return yasd_zend_is_callable_at_frame(
        zcallable, zobject, NULL, check_flags, callable_name, callable_name_len, fci_cache, error);
}

static inline void yasd_zend_fci_cache_discard(zend_fcall_info_cache *fci_cache) {
    if (fci_cache->object) {
        OBJ_RELEASE(fci_cache->object);
    }
    if (fci_cache->function_handler->op_array.fn_flags & ZEND_ACC_CLOSURE) {
        OBJ_RELEASE(ZEND_CLOSURE_OBJECT(fci_cache->function_handler));
    }
}

static inline void yasd_zend_fci_cache_persist(zend_fcall_info_cache *fci_cache) {
    if (fci_cache->object) {
        GC_ADDREF(fci_cache->object);
    }
    if (fci_cache->function_handler->op_array.fn_flags & ZEND_ACC_CLOSURE) {
        GC_ADDREF(ZEND_CLOSURE_OBJECT(fci_cache->function_handler));
    }
}

/* this API can work well when retval is NULL */
static inline int yasd_zend_call_function_ex(
    zval *function_name, zend_fcall_info_cache *fci_cache, uint32_t param_count, zval *params, zval *retval) {
    zend_fcall_info fci;
    zval _retval;
    int ret;

    fci.size = sizeof(fci);
    fci.object = NULL;
    if (!fci_cache || !fci_cache->function_handler) {
        if (!function_name) {
            php_yasd_fatal_error(E_WARNING, "Bad function");
            return FAILURE;
        }
        ZVAL_COPY_VALUE(&fci.function_name, function_name);
    } else {
        ZVAL_UNDEF(&fci.function_name);
    }
    fci.retval = retval ? retval : &_retval;
    fci.param_count = param_count;
    fci.params = params;
#if PHP_VERSION_ID >= 80000
    fci.named_params = NULL;
#else
    fci.no_separation = 0;
#endif

    ret = zend_call_function(&fci, fci_cache);

    if (!retval) {
        zval_ptr_dtor(&_retval);
    }
    return ret;
}

#endif /* PHP_YASD_H_ */
