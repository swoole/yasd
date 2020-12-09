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
#include <functional>

#include "include/debuger_mode_base.h"

namespace yasd {
class CmderDebugger: public DebuggerModeBase {
  private:
    std::string last_cmd;
    int last_list_lineno = 1;
    int listsize = 10;
    std::vector<std::pair<std::string, std::function<int()>>> handlers;

    std::string get_next_cmd();
    int execute_cmd();

  public:

    CmderDebugger();
    ~CmderDebugger();

    void init();
    void handle_request(const char *filename, int lineno);

    int parse_run_cmd();
    int parse_breakpoint_cmd();
    int parse_delete_breakpoint_cmd();
    int parse_info_cmd();
    int parse_step_cmd();
    int parse_level_cmd();
    int parse_backtrace_cmd();
    int parse_next_cmd();
    int parse_continue_cmd();
    int parse_quit_cmd();
    int parse_print_cmd();
    int parse_list_cmd();
    int parse_set_cmd();
    int parse_watch_cmd();
    int parse_unwatch_cmd();
    int parse_finish_cmd();

    bool is_disable_cmd(std::string cmd);
    std::string get_full_name(std::string sub_cmd);

    void register_cmd_handler();
    std::function<int()> find_cmd_handler(std::string cmd);

    void show_welcome_info();
    void show_breakpoint_hit_info();

    int get_listsize() {
        return listsize;
    }

    void set_listsize(int _listsize) {
        listsize = _listsize;
    }
};
}  // namespace yasd
