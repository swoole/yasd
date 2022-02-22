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

#include <sys/file.h>
#include <errno.h>
#include <fcntl.h>  // for O_APPEND in alpine
#include "include/logger.h"
#include "include/common.h"
#include "include/util.h"

namespace yasd {
Logger::Logger(const char *_log_file) {
    log_file = _log_file;

    log_fd = ::open(log_file.c_str(), O_APPEND | O_RDWR | O_CREAT, 0666);
    if (log_fd < 0) {
        printf("open(%s) failed. Error: %s[%d]\n", log_file.c_str(), strerror(errno), errno);
        log_fd = STDOUT_FILENO;
        log_file = "";
    }
}

Logger::~Logger() {
    ::close(log_fd);
    log_fd = STDOUT_FILENO;
    log_file = "";
}

Logger &Logger::set_level(int level) {
    log_level = level;
    return *this;
}

void Logger::put(int level, const char *content, size_t length) {
    int n;

    if (level < log_level) {
        return;
    }

    // TODO(codinghuang): seems there are "buffer overflow detected" problem
    if (length > YASD_MSG_SIZE - 1) {
        yasd::util::printfln_info(yasd::Color::YASD_ECHO_YELLOW, "[yasd] length is greater than the YASD_MSG_SIZE");
        length = YASD_MSG_SIZE - 1;
    }
    memcpy(yasd_info_buf, content, length);
    yasd_info_buf[length] = '\n';
    n = length + 1;

    if (::write(log_fd, yasd_info_buf, n) < 0) {
        printf("write(log_fd=%d, size=%d) failed. Error: %s[%d].\nMessage: %.*s\n",
               log_fd,
               n,
               strerror(errno),
               errno,
               n,
               yasd_info_buf);
    }
}

}  // namespace yasd
