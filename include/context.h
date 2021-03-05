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

#include <string>
#include <vector>
#include <set>

#include "Zend/zend_types.h"

namespace yasd {

class CurrentFunctionStatus {
  public:
    long start_time, end_time;
    int executed_opline_num = 0;
    zend_execute_data *execute_data;

    CurrentFunctionStatus() {}
    ~CurrentFunctionStatus() {}
};

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

    // when a new coroutine is created, the level is reset to 0, so each coroutine needs a level
    int64_t level = 0;
    int64_t next_level = 0;

    std::vector<StackFrame *> *strace = nullptr;
    std::vector<CurrentFunctionStatus *> function_status;

    Context();
    ~Context();
};
}  // namespace yasd
