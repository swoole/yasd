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


#include <map>
#include <string>
#include <vector>
#include <set>

#include "php/main/php.h"

// variable name, zval *
#define WATCHPOINT std::map<std::string, zval *>

namespace yasd {

class WatchPoint {
  public:
    // variable name, zval *
    std::map<zend_function *, WATCHPOINT *> var_watchpoint;

    WatchPoint() {}
    ~WatchPoint() {}
};
}  // namespace yasd
