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
#include <sstream>
#include <utility>

#include "php/main/php.h"

namespace yasd
{
class Util
{
public:
    static std::vector<std::string> explode(std::string const & s, char delim);
    static HashTable *get_defined_vars();
    static void print_var(const char *var_name, size_t var_name_length);
    static void printf_info(int color, const char *format, ...);
    static void show_breakpoint_hit_info();
    static const char *get_executed_filename();
    static int get_executed_file_lineno();
    static const char *get_prev_executed_filename();
    static int get_prev_executed_file_lineno();
    static bool is_match(std::string sub_str, std::string target_str);
};
} // namespace yasd
