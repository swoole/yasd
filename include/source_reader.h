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

#include <map>
#include <string>
#include <fstream>

namespace yasd {
class SourceReader {
  private:
    const char *filename;
    std::fstream file;

    std::map<int, std::string> read_contents(int start, int count, bool show_prev = true);

  public:
    void show_contents(int start_lineno, int line_num, bool point_lineno = false, bool show_prev = false);
    explicit SourceReader(const char *_filename);
    ~SourceReader();
};
}  // namespace yasd
