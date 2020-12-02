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
