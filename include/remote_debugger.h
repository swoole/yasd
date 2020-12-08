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
#include <functional>
#include <vector>
#include <utility>

#include "include/debuger_mode_base.h"
#include "thirdparty/tinyxml2/tinyxml2.h"

namespace yasd {
class RemoteDebugger : public DebuggerModeBase {
  private:
    int sock;
    std::string last_cmd;
    int transaction_id = 0;
    std::vector<std::pair<std::string, std::function<int()>>> handlers;

    std::string get_next_cmd();
    int execute_cmd();

  public:
    RemoteDebugger() {}
    ~RemoteDebugger() {}

    void init();
    void handle_request(const char *filename, int lineno);
    std::string make_message(tinyxml2::XMLDocument *doc);
    ssize_t send_doc(tinyxml2::XMLDocument *doc);

    int parse_breakpoint_list_cmd();
    int parse_breakpoint_set_cmd();
    int parse_breakpoint_set_exception_cmd();
    int parse_run_cmd();
    int parse_stack_get_cmd();
    int parse_context_names_cmd();

    void register_cmd_handler();
    std::function<int()> find_cmd_handler(std::string cmd);
    std::string get_full_name(std::string sub_cmd);

    int breakpoint_admin_add();
};
}  // namespace yasd
