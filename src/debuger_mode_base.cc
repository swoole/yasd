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

    zend_function *func = EG(current_execute_data)->func;

    auto var_watchpoint = global->watchPoints.var_watchpoint.find(func);

    if (var_watchpoint == global->watchPoints.var_watchpoint.end()) {
        return false;
    }

    for (auto watchpointIter = var_watchpoint->second->begin(); watchpointIter != var_watchpoint->second->end();
         watchpointIter++) {
        std::string var_name = watchpointIter->first;
        yasd::WatchPointElement &watchpoint = watchpointIter->second;

        zval *new_var = yasd::util::variable::find_variable(var_name);
        if (new_var == nullptr) {
            zval tmp;
            new_var = &tmp;
            ZVAL_UNDEF(new_var);
        }
        std::string op = watchpoint.operation;
        zval *old_var = &watchpoint.old_var;

        if (watchpoint.type == yasd::WatchPointElement::VARIABLE_CHANGE) {
            zval *old_var = &watchpoint.old_var;

            if (!yasd::util::variable::is_equal(new_var, old_var)) {
                watchpoint.old_var = *new_var;
                return true;
            }
        } else {
            if (op == "<") {
                if (yasd::util::variable::is_smaller(new_var, old_var)) {
                    var_watchpoint->second->erase(watchpointIter);
                    return true;
                }
            } else if (op == ">") {
                if (yasd::util::variable::is_greater(new_var, old_var)) {
                    var_watchpoint->second->erase(watchpointIter);
                    return true;
                }
            } else if (op == "==") {
                if (yasd::util::variable::is_equal(new_var, old_var)) {
                    var_watchpoint->second->erase(watchpointIter);
                    return true;
                }
            }
        }
    }

    return false;
}
}  // namespace yasd
