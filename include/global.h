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

#include "include/context.h"

#include <map>

namespace yasd {
class Global {
  public:
    bool is_running = false;
    bool do_step = false;
    bool do_next = false;

    char *entry_file = nullptr;

    std::map<int, Context *> *contexts;

    // filename, [lineno]
    std::map<BREAKPOINT> *breakpoints;

    Global(/* args */);
    ~Global();

    Context *get_current_context();
};
}  // namespace yasd

extern yasd::Global *global;
extern zend_function *get_cid_function;
