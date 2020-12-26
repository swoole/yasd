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

YASD_EXTERN_C_BEGIN
#include "ext/standard/php_var.h"
YASD_EXTERN_C_END

#include "./php_yasd.h"

namespace yasd {

HashTable *Util::get_defined_vars() {
    HashTable *symbol_table;

    symbol_table = zend_rebuild_symbol_table();

    return symbol_table;
}

zval *Util::find_variable(std::string var_name) {
    HashTable *defined_vars;

    defined_vars = get_defined_vars();

    return find_variable(defined_vars, var_name);
}

zval *Util::find_variable(zend_array *symbol_table, std::string var_name) {
    zval *var;

    if (var_name == "this") {
        return &EG(current_execute_data)->This;
    }

    var = zend_hash_str_find(symbol_table, var_name.c_str(), var_name.length());

    // not define variable
    if (!var) {
        return nullptr;
    }

    while (Z_TYPE_P(var) == IS_INDIRECT) {
        var = Z_INDIRECT_P(var);
    }

    return var;
}

void Util::print_var(std::string fullname) {
    zval *var;

    var = yasd::Util::fetch_zval_by_fullname(fullname);

    if (!var) {
        yasd::Util::printfln_info(yasd::Color::YASD_ECHO_GREEN, "not found variable $%s", fullname.c_str());
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

// create file and clear file
void Util::clear_breakpoint_cache_file() {
    std::string cache_filename_path = yasd::Util::get_breakpoint_cache_filename();

    if (cache_filename_path == "") {
        return;
    }

    std::ofstream file(cache_filename_path);
    file.close();
}

std::string Util::get_breakpoint_cache_filename() {
    return std::string(YASD_G(breakpoints_file));
}

void Util::cache_breakpoint(std::string filename, int lineno) {
    std::ofstream file;
    std::string cache_filename_path = yasd::Util::get_breakpoint_cache_filename();

    if (cache_filename_path == "") {
        return;
    }

    file.open(cache_filename_path, std::ios_base::app);
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
#if PHP_VERSION_ID >= 70400
    return zend_get_properties_for(zobj, ZEND_PROP_PURPOSE_VAR_EXPORT);
#else
    if (Z_OBJ_HANDLER_P(zobj, get_properties)) {
        return Z_OBJPROP_P(zobj);
    }
#endif
    return nullptr;
}

std::string Util::get_property_name(zend_string *property_name) {
    const char *class_name, *_property_name;
    size_t _property_name_len;

    zend_unmangle_property_name_ex(property_name, &class_name, &_property_name, &_property_name_len);

    return std::string(_property_name, _property_name_len);
}

std::string Util::get_option_value(const std::vector<std::string> &options, std::string option) {
    auto iter = options.begin();

    for (; iter != options.end(); iter++) {
        if (option == *iter) {
            break;
        }
    }

    return *(++iter);
}

// TODO(codinghuang): maybe we can use re2c and yacc later
zval *Util::fetch_zval_by_fullname(std::string fullname) {
    std::string name;
    int state = 0;
    const char *ptr = fullname.c_str();
    const char *keyword = ptr, *keyword_end = nullptr;
    const char *end = fullname.c_str() + fullname.length() - 1;
    zval *retval_ptr = nullptr;

    zend_array *symbol_table = zend_rebuild_symbol_table();

    // aa->bb->cc
    // aa[bb][cc]

    auto fetch_next_zval = [](zval *retval_ptr, zend_array *symbol_table, std::string name) -> zval * {
        if (!retval_ptr) {
            retval_ptr = find_variable(symbol_table, name);
        } else if (Z_TYPE_P(retval_ptr) == IS_ARRAY) {
            retval_ptr = find_variable(Z_ARRVAL_P(retval_ptr), name);
        } else {
            retval_ptr = yasd_zend_read_property(Z_OBJCE_P(retval_ptr), retval_ptr, name.c_str(), name.length(), 1);
        }
        return retval_ptr;
    };

    do {
        switch (state) {
        case 0:
            keyword = ptr;
            state = 1;
        case 1:
            if (*ptr == '[') {
                keyword_end = ptr;
                if (keyword) {
                    std::string name(keyword, keyword_end - keyword);
                    retval_ptr = fetch_next_zval(retval_ptr, symbol_table, name);
                    keyword = nullptr;
                }
                state = 3;
            } else if (*ptr == '-') {
                keyword_end = ptr;
                if (keyword) {
                    std::string name(keyword, keyword_end - keyword);
                    retval_ptr = fetch_next_zval(retval_ptr, symbol_table, name);
                    keyword = nullptr;
                }
                state = 2;
            }
            break;
        case 2:
            assert(*ptr == '>');
            keyword = ptr + 1;
            state = 1;
            break;
        case 3:
            if ((*ptr >= 'a' && *ptr <= 'z') || (*ptr >= 'A' && *ptr <= 'Z')) {
                state = 6;
                keyword = ptr;
            } else if (*ptr >= '0' && *ptr <= '9') {
                state = 6;
                keyword = ptr;
            }
            break;
        case 4:
            break;
        case 5:
            if (*ptr == ']') {
                state = 1;
            }
            break;
        case 6:
            if (*ptr == ']') {
                keyword_end = ptr;
                if (keyword) {
                    std::string name(keyword, keyword_end - keyword);
                    retval_ptr = fetch_next_zval(retval_ptr, symbol_table, name);
                    keyword = nullptr;
                }
                state = 1;
            }
            break;
        default:
            break;
        }
        ptr++;
    } while (ptr <= end);

    if (keyword) {
        keyword_end = ptr;
        std::string name(keyword, keyword_end - keyword);

        retval_ptr = fetch_next_zval(retval_ptr, symbol_table, name);
        keyword = nullptr;
    }

    return retval_ptr;
}

}  // namespace yasd
