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
