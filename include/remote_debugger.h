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

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <string>

#include "include/debuger_mode_base.h"
#include "thirdparty/tinyxml2/tinyxml2.h"

namespace yasd {
class RemoteDebugger : public DebuggerModeBase {
  private:
    int sock;

  public:
    RemoteDebugger() {}
    ~RemoteDebugger() {}

    void init();
    void handle_request(const char *filename, int lineno);
    std::string make_message(tinyxml2::XMLDocument *doc);
    ssize_t send_doc(tinyxml2::XMLDocument *doc);
};
}  // namespace yasd
