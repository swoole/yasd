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
#include "include/util.h"
#include "include/debuger_mode_base.h"

namespace yasd {
int DebuggerModeBase::parse_step_over_cmd() {
    // https://xdebug.org/docs/dbgp#continuation-commands

    yasd::Context *context = global->get_current_context();
    zend_execute_data *frame = EG(current_execute_data);

    int func_line_end = frame->func->op_array.line_end;

    if (frame->opline->lineno == func_line_end && context->level != 1) {
        global->do_step = true;
    } else if (frame->opline->lineno == func_line_end && context->level == 1) {
        global->do_step = false;
        global->do_next = false;
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

bool DebuggerModeBase::is_hit_watch_point() {
    if (!EG(current_execute_data)) {
        return false;
    }

    zval retval;
    zend_function *func = EG(current_execute_data)->func;

    auto variable_change_watchpoint = global->watchPoints.variable_change_watchpoint.find(func);
    auto condition_watchpoint = global->watchPoints.condition_watchpoint.find(func);

    if (variable_change_watchpoint != global->watchPoints.variable_change_watchpoint.end()) {
        for (auto watchpoint_iter = variable_change_watchpoint->second->begin(); watchpoint_iter != variable_change_watchpoint->second->end();
            watchpoint_iter++) {
            std::string var_name = watchpoint_iter->first;
            yasd::WatchPointElement &watchpoint = watchpoint_iter->second;

            zval *new_var = yasd::util::variable::find_variable(var_name);
            if (new_var == nullptr) {
                zval tmp;
                new_var = &tmp;
                ZVAL_UNDEF(new_var);
            }
            zval *old_var = &watchpoint.old_var;

            if (!yasd::util::variable::is_equal(new_var, old_var)) {
                watchpoint.old_var = *new_var;
                return true;
            }
        }
    }

    if (condition_watchpoint != global->watchPoints.condition_watchpoint.end()) {
        for (std::string condition : *condition_watchpoint->second) {
            if (!yasd::util::execution::eval_string(const_cast<char *>(condition.c_str()), &retval, nullptr)) {
                return false;
            }

            return Z_TYPE(retval) == IS_TRUE;
        }
    }

    return false;
}
}  // namespace yasd
