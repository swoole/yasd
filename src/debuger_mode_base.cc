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
#include "include/global.h"
#include "include/debuger_mode_base.h"

namespace yasd {
int DebuggerModeBase::parse_step_over_cmd() {
    // https://xdebug.org/docs/dbgp#continuation-commands

    yasd::Context *context = global->get_current_context();
    zend_execute_data *frame = EG(current_execute_data);

    int func_line_end = frame->func->op_array.line_end;

    if (frame->opline->lineno == func_line_end && context->level != 1) {
        global->do_step = true;
    } else {
        context->next_level = context->level;
        global->next_cid = context->cid;
        global->do_next = true;
    }

    return NEXT_OPLINE;
}

int DebuggerModeBase::parse_step_into_cmd() {
    global->do_step = true;

    return NEXT_OPLINE;
}

int DebuggerModeBase::parse_step_out_cmd() {
    yasd::Context *context = global->get_current_context();
    // zend_execute_data *frame = EG(current_execute_data);

    // int func_line_end = frame->func->op_array.line_end;

    context->next_level = context->level - 1;
    global->next_cid = context->cid;
    global->do_finish = true;

    return NEXT_OPLINE;
}
}  // namespace yasd
