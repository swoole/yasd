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
    Context();
    ~Context();
};
}  // namespace yasd
