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

extern zend_module_entry yasd_module_entry;
#define phpext_yasd_ptr &yasd_module_entry

#define PHP_YASD_VERSION "0.3.3-alpha"

ZEND_BEGIN_MODULE_GLOBALS(yasd)
    char *breakpoints_file;
    char *debug_mode;
    char *remote_host;
    uint16_t remote_port;
    uint16_t depth;
    int log_level;

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

#endif /* PHP_YASD_H_ */
