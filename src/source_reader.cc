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
#include <iostream>

#include "include/common.h"
#include "include/util.h"
#include "include/source_reader.h"

namespace yasd {
SourceReader::SourceReader(const char *_filename) {
    filename = _filename;

    file.open(filename, std::ios::in);
}
SourceReader::~SourceReader() {}

std::map<int, std::string> SourceReader::read_contents(int start, int count) {
    int i = 0;
    std::string content;
    std::map<int, std::string> contents;
    // zend_execute_data *frame = EG(current_execute_data);

    // int func_line_start = frame->func->op_array.line_start;
    // int func_line_end = frame->func->op_array.line_end;

    int show_line_start = start - count;
    int show_line_end = start + count;

    while (i < show_line_start && getline(file, content)) {
        i++;
    }
    contents.insert(std::make_pair(i, content));

    while (i <= show_line_end && getline(file, content)) {
        i++;
        contents.insert(std::make_pair(i, content));
    }

    file.close();
    return contents;
}

void SourceReader::show_contents(int start_lineno) {
    std::map<int, std::string> contents = read_contents(start_lineno, 5);
    std::cout << "\n" << filename << std::endl;
    for (const auto &content : contents) {
        if (start_lineno == content.first) {
            yasd::Util::printf_info(YASD_ECHO_GREEN, "%d-->\t%s", content.first, content.second.c_str());
        } else {
            std::cout << content.first << "\t" << content.second << std::endl;
        }
    }
}

}  // namespace yasd
