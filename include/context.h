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

#include "php/main/php.h"

#include <map>
#include <string>
#include <vector>
#include <set>

#define BREAKPOINT std::string, std::set<int>

namespace yasd {

class StackFrame {
  public:
    std::string filename;
    std::string function_name;
    int level;
    int lineno;

    StackFrame();
    ~StackFrame();
};

class Context {
  public:
    int64_t cid;
    int64_t level = 0;
    int64_t next_level = 0;

    std::vector<StackFrame *> *strace;

    // variable name, zval *
    std::map<std::string, zval *> watchpoints;
    Context();
    ~Context();
};
}  // namespace yasd
