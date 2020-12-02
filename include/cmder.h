/**
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <string>
#include <functional>

namespace yasd {
class Cmder {
  private:
    std::string last_cmd;
    std::map<std::string, std::function<int()>> handlers;

  public:
    enum status {
        SUCCESS = 0,
        FAILED,
        NEXT_OPLINE,
        RECV_CMD_AGAIN,
    };

    Cmder();
    ~Cmder();

    std::string get_next_cmd();
    int execute_cmd();

    int parse_run_cmd();
    int parse_breakpoint_cmd();
    int parse_delete_breakpoint_cmd();
    int parse_info_cmd();
    int parse_step_cmd();
    int parse_level_cmd();
    int parse_trace_cmd();
    int parse_next_cmd();
    int parse_continue_cmd();
    int parse_quit_cmd();
    int parse_print_cmd();
    int parse_finish_cmd();

    void register_cmd_handler();
    std::function<int()> find_cmd_handler(std::string cmd);

    void show_welcome_info();
    void show_breakpoint_hit_info();
};
}  // namespace yasd

extern yasd::Cmder *cmder;
