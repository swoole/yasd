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

#include <unistd.h>
#include <string>

namespace yasd {
enum LogLevel {
    DEBUG = 0,
    TRACE,
    INFO,
    NOTICE,
    WARNING,
    ERROR,
    NONE,
};
class Logger {
 private:
    int log_fd = STDOUT_FILENO;
    std::string log_file = "";
    int log_level = INFO;

 public:
    explicit Logger(const char *_logfile = nullptr);
    ~Logger();

    Logger &set_level(int level);
    void put(int level, const char *content, size_t length);
};

}  // namespace yasd
