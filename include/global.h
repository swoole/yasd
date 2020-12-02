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

#include "include/context.h"
#include "include/redirect_file_to_cin.h"

#include <map>

namespace yasd {
class Global {
  public:
    yasd::RedirectFileToCin *redirector = nullptr;
    bool is_running = false;
    bool do_step = false;
    bool do_next = false;
    bool do_finish = false;

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
