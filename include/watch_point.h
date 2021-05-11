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

#include "main/php.h"

namespace yasd {

class WatchPointElement {
  public:
    zval old_var;
};

#define VARIABLE_CHANGE_WATCH_POINT std::map<std::string, WatchPointElement>
#define CONDITION_WATCH_POINT std::set<std::string>

class WatchPoint {
  public:
    // variable name, zval *
    std::map<zend_function *, VARIABLE_CHANGE_WATCH_POINT *> variable_change_watchpoint;
    std::map<zend_function *, CONDITION_WATCH_POINT *> condition_watchpoint;

    WatchPoint() {}
    ~WatchPoint() {}
};
}  // namespace yasd
