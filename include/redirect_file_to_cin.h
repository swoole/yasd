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

#include <iostream>
#include <fstream>

namespace yasd {
struct RedirectFileToCin {
    explicit RedirectFileToCin(const char *file_name) {
        fbuf.open(file_name, std::ios::in);             // open file for input
        oldbuf = std::cin.rdbuf(std::addressof(fbuf));  // redirect file to std::cin
    }

    ~RedirectFileToCin() {
        std::cin.rdbuf(oldbuf);
    }

    // not copyable or assignable
    RedirectFileToCin(const RedirectFileToCin &) = delete;
    void operator=(const RedirectFileToCin &) = delete;

    std::filebuf fbuf;
    std::streambuf *oldbuf;
};
}  // namespace yasd
