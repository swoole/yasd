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
#include "include/common.h"
#include "include/context.h"
#include "include/cmder.h"
#include "include/util.h"
#include "include/global.h"
#include "include/source_reader.h"

#include "main/php.h"
#include "Zend/zend_builtin_functions.h"

BEGIN_EXTERN_C()
#include "ext/standard/php_var.h"
END_EXTERN_C()

#include <iostream>

yasd::Cmder *cmder;

namespace yasd {

Cmder::Cmder() {
    register_cmd_handler();
}

Cmder::~Cmder() {}

std::string Cmder::get_next_cmd() {
    std::string tmp;

    std::cout << "> ";
    getline(std::cin, tmp);
    if (tmp == "") {
        return last_cmd;
    }
    last_cmd = tmp;
    return last_cmd;
}

int Cmder::parse_run_cmd() {
    global->is_running = true;

    return NEXT_OPLINE;
}

int Cmder::parse_breakpoint_cmd() {
    int lineno;
    std::string filename;

    auto exploded_cmd = yasd::Util::explode(last_cmd, " ");

    // breakpoint in current file with lineno
    if (exploded_cmd.size() == 2) {
        filename = yasd::Util::get_executed_filename();
        if (filename == "") {
            filename = global->entry_file;
        }
        lineno = atoi(exploded_cmd[1].c_str());
    } else if (exploded_cmd.size() == 3) {
        filename = exploded_cmd[1];
        lineno = atoi(exploded_cmd[2].c_str());
    } else {
        yasd::Util::printfln_info(YASD_ECHO_RED, "use set breakpoint cmd error!");
        return RECV_CMD_AGAIN;
    }

    auto iter = global->breakpoints->find(filename);

    if (iter != global->breakpoints->end()) {
        iter->second.insert(lineno);
    } else {
        std::set<int> lineno_set;
        lineno_set.insert(lineno);
        global->breakpoints->insert(std::make_pair(filename, lineno_set));
    }

    yasd::Util::cache_breakpoint(filename, lineno);

    yasd::Util::printfln_info(yasd::Color::YASD_ECHO_GREEN, "set breakpoint at %s:%d", filename.c_str(), lineno);

    return RECV_CMD_AGAIN;
}

int Cmder::parse_delete_breakpoint_cmd() {
    int lineno;
    std::string filename;

    auto exploded_cmd = yasd::Util::explode(last_cmd, " ");

    if (exploded_cmd.size() == 2) {
        filename = yasd::Util::get_executed_filename();
        if (filename == "") {
            filename = global->entry_file;
        }
        lineno = atoi(exploded_cmd[1].c_str());
    } else if (exploded_cmd.size() == 3) {
        filename = exploded_cmd[1];
        lineno = atoi(exploded_cmd[2].c_str());
    } else {
        yasd::Util::printfln_info(YASD_ECHO_RED, "use delete breakpoint cmd error!");
        return RECV_CMD_AGAIN;
    }

    auto iter = global->breakpoints->find(filename);

    if (iter != global->breakpoints->end()) {
        iter->second.erase(lineno);
        if (iter->second.empty()) {
            global->breakpoints->erase(iter->first);
        }
        yasd::Util::printfln_info(yasd::Color::YASD_ECHO_GREEN, "delete breakpoint at %s:%d", filename.c_str(), lineno);
    } else {
        yasd::Util::printfln_info(YASD_ECHO_RED, "breakpoint at %s:%d is not existed!", filename.c_str(), lineno);
    }

    return RECV_CMD_AGAIN;
}

int Cmder::parse_info_cmd() {
    if (global->breakpoints->empty()) {
        yasd::Util::printfln_info(YASD_ECHO_RED, "no found breakpoints!");
    }
    for (auto i = global->breakpoints->begin(); i != global->breakpoints->end(); i++) {
        for (auto j = i->second.begin(); j != i->second.end(); j++) {
            yasd::Util::printfln_info(yasd::Color::YASD_ECHO_GREEN, "filename: %s:%d", i->first.c_str(), *j);
        }
    }

    return RECV_CMD_AGAIN;
}

int Cmder::parse_step_cmd() {
    global->do_step = true;

    return NEXT_OPLINE;
}

int Cmder::parse_level_cmd() {
    yasd::Context *context = global->get_current_context();

    std::cout << "in coroutine: " << context->cid << ", level: " << context->level
              << ", next level: " << context->next_level << std::endl;

    return RECV_CMD_AGAIN;
}

int Cmder::parse_backtrace_cmd() {
    yasd::Context *context = global->get_current_context();

    yasd::Util::printfln_info(
        YASD_ECHO_GREEN, "%s:%d", yasd::Util::get_executed_filename(), yasd::Util::get_executed_file_lineno());

    for (auto iter = context->strace->rbegin(); iter != context->strace->rend(); ++iter) {
        yasd::Util::printfln_info(YASD_ECHO_GREEN, "%s:%d", (*iter)->filename.c_str(), (*iter)->lineno);
    }

    return RECV_CMD_AGAIN;
}

int Cmder::parse_next_cmd() {
    yasd::Context *context = global->get_current_context();
    zend_execute_data *frame = EG(current_execute_data);

    int func_line_end = frame->func->op_array.line_end;

    if (frame->opline->lineno == func_line_end) {
        global->do_step = true;
    } else {
        context->next_level = context->level;
        global->do_next = true;
    }

    return NEXT_OPLINE;
}

int Cmder::parse_continue_cmd() {
    return NEXT_OPLINE;
}

int Cmder::parse_quit_cmd() {
    yasd::Util::printfln_info(YASD_ECHO_RED, "quit!");
    exit(255);

    return FAILED;
}

int Cmder::parse_print_cmd() {
    auto exploded_cmd = yasd::Util::explode(last_cmd, " ");

    yasd::Util::print_var(exploded_cmd[1]);
    global->do_next = true;

    return RECV_CMD_AGAIN;
}

int Cmder::parse_list_cmd() {
    int lineno = last_list_lineno;
    const char *filename = yasd::Util::get_executed_filename();
    yasd::SourceReader reader(filename);

    auto exploded_cmd = yasd::Util::explode(last_cmd, " ");

    // list lineno or list -
    if (exploded_cmd.size() == 2) {
        if (exploded_cmd[1] == "-") {
            lineno = last_list_lineno - listsize;
        } else {
            lineno = atoi(exploded_cmd[1].c_str());
            lineno = last_list_lineno + listsize;
        }
        last_list_lineno = lineno;
    } else {
        // list
        lineno = last_list_lineno;
        last_list_lineno = lineno + listsize;
    }

    reader.show_contents(lineno, cmder->get_listsize());
    return RECV_CMD_AGAIN;
}

int Cmder::parse_set_cmd() {
    auto exploded_cmd = yasd::Util::explode(last_cmd, " ");

    if (exploded_cmd[1] == "listsize") {
        listsize = atoi(exploded_cmd[2].c_str());
    }
    return RECV_CMD_AGAIN;
}

int Cmder::parse_watch_cmd() {
    zend_function *func = EG(current_execute_data)->func;

    auto exploded_cmd = yasd::Util::explode(last_cmd, " ");
    if (exploded_cmd.size() < 2) {
        yasd::Util::printfln_info(yasd::Color::YASD_ECHO_RED, "you should set watch point");
        return RECV_CMD_AGAIN;
    }
    std::string var_name = exploded_cmd[1];

    zval *var = yasd::Util::find_variable(var_name);

    if (!var) {
        zval tmp;
        var = &tmp;
        ZVAL_UNDEF(var);
    }

    auto iter = global->watchPoints.var_watchpoint.find(func);
    if (iter == global->watchPoints.var_watchpoint.end()) {
        WATCHPOINT *watchpoint = new WATCHPOINT();
        watchpoint->insert(std::make_pair(var_name, *var));
        global->watchPoints.var_watchpoint.insert(std::make_pair(func, watchpoint));
    } else {
        iter->second->insert(std::make_pair(var_name, *var));
    }

    yasd::Util::printfln_info(yasd::Color::YASD_ECHO_GREEN, "watching variable $%s", var_name.c_str());

    return RECV_CMD_AGAIN;
}

int Cmder::parse_unwatch_cmd() {
    zend_function *func = EG(current_execute_data)->func;

    auto exploded_cmd = yasd::Util::explode(last_cmd, " ");
    if (exploded_cmd.size() < 2) {
        yasd::Util::printfln_info(yasd::Color::YASD_ECHO_RED, "you should set watch point");
        return RECV_CMD_AGAIN;
    }
    std::string var_name = exploded_cmd[1];

    auto iter = global->watchPoints.var_watchpoint.find(func);
    if (iter == global->watchPoints.var_watchpoint.end()) {
        yasd::Util::printfln_info(yasd::Color::YASD_ECHO_GREEN, "not found watch point $%s", var_name.c_str());
    } else {
        auto zval_iter = iter->second->find(var_name);
        if (zval_iter == iter->second->end()) {
            yasd::Util::printfln_info(yasd::Color::YASD_ECHO_GREEN, "not found watch point $%s", var_name.c_str());
        } else {
            zval_dtor(&zval_iter->second);
            iter->second->erase(var_name);
            yasd::Util::printfln_info(yasd::Color::YASD_ECHO_GREEN, "unwatch variable $%s", var_name.c_str());
        }
    }

    return RECV_CMD_AGAIN;
}

int Cmder::parse_finish_cmd() {
    yasd::Context *context = global->get_current_context();
    // zend_execute_data *frame = EG(current_execute_data);

    // int func_line_end = frame->func->op_array.line_end;

    context->next_level = context->level - 1;
    global->do_finish = true;

    return NEXT_OPLINE;
}

bool Cmder::is_disable_cmd(std::string cmd) {
    // the command in the condition is allowed to execute in non-run state
    if (get_full_name(cmd) != "run" && get_full_name(cmd) != "b" && get_full_name(cmd) != "quit" &&
        get_full_name(cmd) != "info" && get_full_name(cmd) != "delete" && get_full_name(cmd) != "list" &&
        get_full_name(cmd) != "set") {
        return true;
    }

    return false;
}

std::string Cmder::get_full_name(std::string sub_cmd) {
    for (auto &&kv : handlers) {
        if (yasd::Util::is_match(sub_cmd, kv.first)) {
            return kv.first;
        }
    }
    return "unknown cmd";
}

void Cmder::show_welcome_info() {
    yasd::Util::printfln_info(YASD_ECHO_GREEN, "[Welcome to yasd, the Swoole debugger]");
    yasd::Util::printfln_info(YASD_ECHO_GREEN, "[You can set breakpoint now]");
}

int Cmder::execute_cmd() {
    // yasd::Context *context = global->get_current_context();

    auto exploded_cmd = yasd::Util::explode(last_cmd, " ");

    if (!global->is_running) {
        if (is_disable_cmd(exploded_cmd[0])) {
            yasd::Util::printfln_info(YASD_ECHO_RED, "program is not running!");
            return RECV_CMD_AGAIN;
        }
    }

    auto handler = find_cmd_handler(exploded_cmd[0]);
    if (!handler) {
        return FAILED;
    }

    return handler();
}

void Cmder::register_cmd_handler() {
    handlers.push_back(std::make_pair("run", std::bind(&Cmder::parse_run_cmd, this)));
    handlers.push_back(std::make_pair("b", std::bind(&Cmder::parse_breakpoint_cmd, this)));
    handlers.push_back(std::make_pair("bt", std::bind(&Cmder::parse_backtrace_cmd, this)));
    handlers.push_back(std::make_pair("delete", std::bind(&Cmder::parse_delete_breakpoint_cmd, this)));
    handlers.push_back(std::make_pair("info", std::bind(&Cmder::parse_info_cmd, this)));
    handlers.push_back(std::make_pair("step", std::bind(&Cmder::parse_step_cmd, this)));
    handlers.push_back(std::make_pair("list", std::bind(&Cmder::parse_list_cmd, this)));
    handlers.push_back(std::make_pair("next", std::bind(&Cmder::parse_next_cmd, this)));
    handlers.push_back(std::make_pair("continue", std::bind(&Cmder::parse_continue_cmd, this)));
    handlers.push_back(std::make_pair("quit", std::bind(&Cmder::parse_quit_cmd, this)));
    handlers.push_back(std::make_pair("print", std::bind(&Cmder::parse_print_cmd, this)));
    handlers.push_back(std::make_pair("finish", std::bind(&Cmder::parse_finish_cmd, this)));
    handlers.push_back(std::make_pair("set", std::bind(&Cmder::parse_set_cmd, this)));
    handlers.push_back(std::make_pair("level", std::bind(&Cmder::parse_level_cmd, this)));
    handlers.push_back(std::make_pair("watch", std::bind(&Cmder::parse_watch_cmd, this)));
    handlers.push_back(std::make_pair("unwatch", std::bind(&Cmder::parse_unwatch_cmd, this)));
}

std::function<int()> Cmder::find_cmd_handler(std::string cmd) {
    for (auto &&kv : handlers) {
        if (kv.first == get_full_name(cmd)) {
            return kv.second;
        }
    }
    return nullptr;
}
}  // namespace yasd
