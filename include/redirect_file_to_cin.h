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
