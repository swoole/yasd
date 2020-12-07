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
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <netinet/in.h>

#include "include/debuger_mode_base.h"

namespace yasd {
class RemoteDebugger: public DebuggerModeBase {
  private:
    int sock;
  public:
    RemoteDebugger() {}
    ~RemoteDebugger() {}

    void init();
    void handle_request(const char *filename, int lineno);
};
}  // namespace yasd
