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
#include "include/logger.h"
#include "include/redirect_file_to_cin.h"
#include "include/watch_point.h"
#include "include/debuger_mode_base.h"

#include <map>

namespace yasd {
class Global {
  public:
    yasd::RedirectFileToCin *redirector = nullptr;
    bool is_running = false;
    bool do_step = false;
    bool do_next = false;
    bool do_finish = false;

    bool first_entry = true;

    bool is_detach = false;

    // because cid does not repeat, next_cid can be global
    int64_t next_cid = 0;

    int breakpoint_count = 0;

    DebuggerModeBase *debugger = nullptr;

    std::map<int, Context *> *contexts;

    // filename, [lineno]
    std::map<BREAKPOINT> *breakpoints;

    yasd::WatchPoint watchPoints;

    yasd::Logger *logger = nullptr;

    Global();
    ~Global();

    Context *get_current_context();
};
}  // namespace yasd

extern yasd::Global *global;
extern zend_function *get_cid_function;
