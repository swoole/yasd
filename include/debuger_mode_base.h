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

namespace yasd {
class DebuggerModeBase {
  public:
    enum status {
        SUCCESS = 0,
        FAILED,
        NEXT_OPLINE,
        RECV_CMD_AGAIN,
    };

    DebuggerModeBase() {}
    virtual ~DebuggerModeBase() {}

    int parse_step_over_cmd();
    int parse_step_into_cmd();
    int parse_step_out_cmd();

    virtual void init() = 0;
    virtual void handle_request(const char *filename, int lineno) = 0;
};
}  // namespace yasd
