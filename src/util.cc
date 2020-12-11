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

#include <iostream>

#include "include/util.h"
#include "include/common.h"
#include "include/global.h"

BEGIN_EXTERN_C()
#include "ext/standard/php_var.h"
END_EXTERN_C()

#include "./php_yasd.h"

namespace yasd {

std::vector<std::string> Util::explode(const std::string &target, const std::string &delimiter) {
    std::vector<std::string> arr;

    int str_len = target.length();
    int del_len = delimiter.length();

    if (del_len == 0) {
        return arr;
    }

    int i = 0;
    int k = 0;

    while (i < str_len) {
        int j = 0;
        while (i + j < str_len && j < del_len && target[i + j] == delimiter[j]) {
            j++;
        }

        if (j == del_len) {
            arr.emplace_back(target.substr(k, i - k));
            i += del_len;
            k = i;
        } else {
            i++;
        }
    }

    arr.emplace_back(target.substr(k, i - k));
    return arr;
}

HashTable *Util::get_defined_vars() {
    HashTable *symbol_table;

    symbol_table = zend_rebuild_symbol_table();

    return symbol_table;
}

zval *Util::find_variable(std::string var_name) {
    zval *var;
    HashTable *defined_vars;

    defined_vars = get_defined_vars();

    var = zend_hash_str_find(defined_vars, var_name.c_str(), var_name.length());

    // not define variable
    if (!var) {
        return nullptr;
    }

    while (Z_TYPE_P(var) == IS_INDIRECT) {
        var = Z_INDIRECT_P(var);
    }

    // the statement that defines the variable has not yet been executed
    if (Z_TYPE_P(var) == IS_UNDEF) {
        return nullptr;
    }

    return var;
}

void Util::print_var(std::string var_name) {
    zval *var;
    zend_execute_data *execute_data = EG(current_execute_data);

    // print $this
    if (var_name == "this") {
        php_var_dump(ZEND_THIS, 1);
        return;
    }

    // print property
    auto exploded_var_name = yasd::Util::explode(var_name, "->");
    if (exploded_var_name.size() == 2) {
        print_property(exploded_var_name[0], exploded_var_name[1]);
        return;
    }

    var = find_variable(var_name);

    if (!var) {
        yasd::Util::printfln_info(yasd::Color::YASD_ECHO_GREEN, "not found variable $%s", var_name.c_str());
    } else {
        php_var_dump(var, 1);
    }
}

void Util::print_property(std::string obj_name, std::string property_name) {
    zval *obj;
    zval *property;
    zend_execute_data *execute_data = EG(current_execute_data);

    if (obj_name == "this") {
        property =
            yasd_zend_read_property(Z_OBJCE_P(ZEND_THIS), ZEND_THIS, property_name.c_str(), property_name.length(), 1);
        php_var_dump(property, 1);
    } else {
        obj = find_variable(obj_name);
        if (!obj) {
            yasd::Util::printfln_info(yasd::Color::YASD_ECHO_RED, "undefined variable %s", obj_name.c_str());
            return;
        }
        property = yasd_zend_read_property(Z_OBJCE_P(obj), obj, property_name.c_str(), property_name.length(), 1);
        if (!property) {
            yasd::Util::printfln_info(yasd::Color::YASD_ECHO_GREEN,
                                      "undefined property %s::$%s",
                                      ZSTR_VAL(Z_OBJCE_P(obj)->name),
                                      property_name.c_str());
            return;
        }
        php_var_dump(property, 1);
    }

    return;
}

void Util::printf_info(int color, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(yasd_info_buf, sizeof(yasd_info_buf), format, args);
    va_end(args);

    switch (color) {
    case yasd::Color::YASD_ECHO_RED:
        std::cout << "\e[31m" << yasd_info_buf << "\e[0m";
        break;
    case yasd::Color::YASD_ECHO_GREEN:
        std::cout << "\e[32m" << yasd_info_buf << "\e[0m";
        break;
    case yasd::Color::YASD_ECHO_YELLOW:
        std::cout << "\e[33m" << yasd_info_buf << "\e[0m";
        break;
    case yasd::Color::YASD_ECHO_BLUE:
        std::cout << "\e[34m" << yasd_info_buf << "\e[0m";
        break;
    case yasd::Color::YASD_ECHO_MAGENTA:
        std::cout << "\e[35m" << yasd_info_buf << "\e[0m";
        break;
    case yasd::Color::YASD_ECHO_CYAN:
        std::cout << "\e[36m" << yasd_info_buf << "\e[0m";
        break;
    case yasd::Color::YASD_ECHO_WHITE:
        std::cout << "\e[37m" << yasd_info_buf << "\e[0m";
        break;
    default:
        break;
    }
}

void Util::show_breakpoint_hit_info() {
    printf_info(YASD_ECHO_GREEN, "stop at breakponit ");
}

const char *Util::get_executed_filename() {
    zend_string *filename;

    filename = zend_get_executed_filename_ex();

    if (UNEXPECTED(filename == nullptr)) {
        return global->entry_file;
    }

    return ZSTR_VAL(filename);
}

const char *Util::get_executed_function_name() {
    zend_execute_data *ptr = EG(current_execute_data);

    while (ptr && (!ptr->func || !ZEND_USER_CODE(ptr->func->type))) {
        ptr = ptr->prev_execute_data;
    }

    if (ptr && ptr->func && ptr->func->op_array.function_name) {
        return ZSTR_VAL(ptr->func->op_array.function_name);
    }
    return "main";
}

int Util::get_executed_file_lineno() {
    if (!EG(current_execute_data)) {
        return 0;
    }
    return EG(current_execute_data)->opline->lineno;
}

const char *Util::get_prev_executed_filename() {
    zend_execute_data *ptr = EG(current_execute_data)->prev_execute_data;

    while (ptr && (!ptr->func || !ZEND_USER_CODE(ptr->func->type))) {
        ptr = ptr->prev_execute_data;
    }

    if (ptr && ptr->func) {
        return ptr->func->op_array.filename->val;
    }

    return "unknow file";
}

const char *Util::get_prev_executed_function_name() {
    zend_execute_data *ptr = EG(current_execute_data)->prev_execute_data;

    while (ptr && (!ptr->func || !ZEND_USER_CODE(ptr->func->type))) {
        ptr = ptr->prev_execute_data;
    }

    if (ptr && ptr->func && ptr->func->op_array.function_name) {
        return ZSTR_VAL(ptr->func->op_array.function_name);
    }
    return "main";
}

int Util::get_prev_executed_file_lineno() {
    zend_execute_data *ptr = EG(current_execute_data)->prev_execute_data;

    while (ptr && (!ptr->func || !ZEND_USER_CODE(ptr->func->type))) {
        ptr = ptr->prev_execute_data;
    }

    if (ptr && ptr->opline) {
        return ptr->opline->lineno;
    }

    return 0;
}

bool Util::is_match(std::string sub_str, std::string target_str) {
    for (size_t i = 0; i < sub_str.length(); i++) {
        if (sub_str[i] != target_str[i]) {
            return false;
        }
    }

    return true;
}

void Util::reload_cache_breakpoint() {
    std::string content;
    std::fstream file(yasd::Util::get_breakpoint_cache_filename());
    std::string filename;
    int lineno;

    while (getline(file, content)) {
        auto exploded_content = explode(content, ":");
        if (exploded_content.size() != 2) {
            continue;
        }

        filename = exploded_content[0];
        lineno = atoi(exploded_content[1].c_str());

        auto iter = global->breakpoints->find(filename);

        yasd::Util::printfln_info(yasd::Color::YASD_ECHO_GREEN, "reload breakpoint at %s:%d", filename.c_str(), lineno);

        if (iter != global->breakpoints->end()) {
            iter->second.insert(lineno);
        } else {
            std::set<int> lineno_set;
            lineno_set.insert(lineno);
            global->breakpoints->insert(std::make_pair(filename, lineno_set));
        }
    }
}

void Util::clear_breakpoint_cache_file() {
    // create file and clear file
    std::ofstream file(yasd::Util::get_breakpoint_cache_filename());
    file.close();
}

std::string Util::get_breakpoint_cache_filename() {
    return std::string(YASD_G(breakpoints_file));
}

void Util::cache_breakpoint(std::string filename, int lineno) {
    std::ofstream file;
    std::string path = get_breakpoint_cache_filename();

    file.open(path, std::ios_base::app);
    file << filename + ":" + std::to_string(lineno) + "\n";
    file.close();
}

bool Util::is_variable_equal(zval *op1, zval *op2) {
    zval result;

    is_equal_function(&result, op1, op2);
    return Z_LVAL(result) == 0;
}

bool Util::is_variable_smaller(zval *op1, zval *op2) {
    zval result;
    is_smaller_function(&result, op1, op2);
    return zend_is_true(&result) == 1;
}

bool Util::is_variable_greater(zval *op1, zval *op2) {
    zval result;
    is_smaller_or_equal_function(&result, op1, op2);
    return zend_is_true(&result) == 0;
}

bool Util::is_hit_watch_point() {
    zend_function *func = EG(current_execute_data)->func;

    auto var_watchpoint = global->watchPoints.var_watchpoint.find(func);

    if (var_watchpoint == global->watchPoints.var_watchpoint.end()) {
        return false;
    }

    for (auto watchpointIter = var_watchpoint->second->begin(); watchpointIter != var_watchpoint->second->end();
         watchpointIter++) {
        std::string var_name = watchpointIter->first;
        yasd::WatchPointElement &watchpoint = watchpointIter->second;

        zval *new_var = yasd::Util::find_variable(var_name);
        if (new_var == nullptr) {
            zval tmp;
            new_var = &tmp;
            ZVAL_UNDEF(new_var);
        }
        std::string op = watchpoint.operation;
        zval *old_var = &watchpoint.old_var;

        if (watchpoint.type == yasd::WatchPointElement::VARIABLE_CHANGE) {
            zval *old_var = &watchpoint.old_var;

            if (!yasd::Util::is_variable_equal(new_var, old_var)) {
                watchpoint.old_var = *new_var;
                return true;
            }
        } else {
            if (op == "<") {
                if (yasd::Util::is_variable_smaller(new_var, old_var)) {
                    var_watchpoint->second->erase(watchpointIter);
                    return true;
                }
            } else if (op == ">") {
                if (yasd::Util::is_variable_greater(new_var, old_var)) {
                    var_watchpoint->second->erase(watchpointIter);
                    return true;
                }
            } else if (op == "==") {
                if (yasd::Util::is_variable_equal(new_var, old_var)) {
                    var_watchpoint->second->erase(watchpointIter);
                    return true;
                }
            }
        }
    }

    return false;
}

bool Util::is_integer(const std::string &s) {
    if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;

    char *p;
    strtol(s.c_str(), &p, 10);

    return (*p == 0);
}

bool Util::eval(char *str, zval *retval_ptr, char *string_name) {
    int origin_error_reporting = EG(error_reporting);

    // we need to turn off the warning if the variable is UNDEF
    EG(error_reporting) = 0;

    int ret;
    ret = zend_eval_string(str, retval_ptr, const_cast<char *>("xdebug://debug-eval"));
    if (ret == FAILURE) {
        return false;
    }

    EG(error_reporting) = origin_error_reporting;

    return true;
}

zend_array *Util::get_properties(zval *zobj) {
    if (Z_OBJ_HANDLER_P(zobj, get_properties)) {
        return Z_OBJPROP_P(zobj);
    }
    return nullptr;
}

}  // namespace yasd
