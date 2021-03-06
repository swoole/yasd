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
#include "include/global.h"

#include "./php_yasd_cxx.h"

YASD_EXTERN_C_BEGIN
#include "ext/standard/php_var.h"
#include "ext/standard/php_string.h"
YASD_EXTERN_C_END

namespace yasd {

HashTable *util::get_defined_vars() {
    HashTable *symbol_table;

    symbol_table = zend_rebuild_symbol_table();

    return symbol_table;
}

zval *util::find_variable(std::string var_name) {
    HashTable *defined_vars;

    defined_vars = get_defined_vars();

    return find_variable(defined_vars, var_name);
}

zval *util::find_variable(zend_array *symbol_table, zend_ulong index) {
    zval *var;

    var = zend_hash_index_find(symbol_table, index);

    // not define variable
    if (!var) {
        return nullptr;
    }

    while (Z_TYPE_P(var) == IS_INDIRECT) {
        var = Z_INDIRECT_P(var);
    }

    return var;
}

zval *util::find_variable(zend_array *symbol_table, std::string var_name) {
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

void util::print_var(std::string fullname) {
    zval *var;

    var = yasd::util::fetch_zval_by_fullname(fullname);

    if (!var) {
        yasd::util::printfln_info(yasd::Color::YASD_ECHO_GREEN, "not found variable $%s", fullname.c_str());
    } else {
        php_var_dump(var, 1);
    }
}

void util::print_property(std::string obj_name, std::string property_name) {
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
            yasd::util::printfln_info(yasd::Color::YASD_ECHO_RED, "undefined variable %s", obj_name.c_str());
            return;
        }
        property = yasd_zend_read_property(Z_OBJCE_P(obj), obj, property_name.c_str(), property_name.length(), 1);
        if (!property) {
            yasd::util::printfln_info(yasd::Color::YASD_ECHO_GREEN,
                                      "undefined property %s::$%s",
                                      ZSTR_VAL(Z_OBJCE_P(obj)->name),
                                      property_name.c_str());
            return;
        }
        php_var_dump(property, 1);
    }

    return;
}

void util::printf_info(int color, const char *format, ...) {
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

void util::show_breakpoint_hit_info() {
    printf_info(YASD_ECHO_GREEN, "stop at breakponit ");
}

const char *util::get_executed_filename() {
    zend_string *filename;

    filename = zend_get_executed_filename_ex();

    return ZSTR_VAL(filename);
}

const char *util::get_executed_function_name() {
    zend_execute_data *ptr = EG(current_execute_data);

    while (ptr && (!ptr->func || !ZEND_USER_CODE(ptr->func->type))) {
        ptr = ptr->prev_execute_data;
    }

    if (ptr && ptr->func && ptr->func->op_array.function_name) {
        return ZSTR_VAL(ptr->func->op_array.function_name);
    }
    return "main";
}

int util::get_executed_file_lineno() {
    if (!EG(current_execute_data)) {
        return 0;
    }
    return EG(current_execute_data)->opline->lineno;
}

const char *util::get_prev_executed_filename() {
    zend_execute_data *ptr = EG(current_execute_data)->prev_execute_data;

    while (ptr && (!ptr->func || !ZEND_USER_CODE(ptr->func->type))) {
        ptr = ptr->prev_execute_data;
    }

    if (ptr && ptr->func) {
        return ptr->func->op_array.filename->val;
    }

    return "unknow file";
}

const char *util::get_prev_executed_function_name() {
    zend_execute_data *ptr = EG(current_execute_data)->prev_execute_data;

    while (ptr && (!ptr->func || !ZEND_USER_CODE(ptr->func->type))) {
        ptr = ptr->prev_execute_data;
    }

    if (ptr && ptr->func && ptr->func->op_array.function_name) {
        return ZSTR_VAL(ptr->func->op_array.function_name);
    }
    return "main";
}

int util::get_prev_executed_file_lineno() {
    zend_execute_data *ptr = EG(current_execute_data)->prev_execute_data;

    while (ptr && (!ptr->func || !ZEND_USER_CODE(ptr->func->type))) {
        ptr = ptr->prev_execute_data;
    }

    if (ptr && ptr->opline) {
        return ptr->opline->lineno;
    }

    return 0;
}

bool util::is_match(std::string sub_str, std::string target_str) {
    for (size_t i = 0; i < sub_str.length(); i++) {
        if (sub_str[i] != target_str[i]) {
            return false;
        }
    }

    return true;
}

// create file and clear file
void util::clear_breakpoint_cache_file() {
    std::string cache_filename_path = yasd::util::get_breakpoint_cache_filename();

    if (cache_filename_path == "") {
        return;
    }

    std::ofstream file(cache_filename_path);
    file.close();
}

std::string util::get_breakpoint_cache_filename() {
    return std::string(YASD_G(breakpoints_file));
}

void util::cache_breakpoint(std::string filename, int lineno) {
    std::ofstream file;
    std::string cache_filename_path = yasd::util::get_breakpoint_cache_filename();

    if (cache_filename_path == "") {
        return;
    }

    file.open(cache_filename_path, std::ios_base::app);
    file << filename + ":" + std::to_string(lineno) + "\n";
    file.close();
}

bool util::is_variable_equal(zval *op1, zval *op2) {
    zval result;

    is_equal_function(&result, op1, op2);
    return Z_LVAL(result) == 0;
}

bool util::is_variable_smaller(zval *op1, zval *op2) {
    zval result;
    is_smaller_function(&result, op1, op2);
    return zend_is_true(&result) == 1;
}

bool util::is_variable_greater(zval *op1, zval *op2) {
    zval result;
    is_smaller_or_equal_function(&result, op1, op2);
    return zend_is_true(&result) == 0;
}

bool util::is_hit_watch_point() {
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

        zval *new_var = yasd::util::find_variable(var_name);
        if (new_var == nullptr) {
            zval tmp;
            new_var = &tmp;
            ZVAL_UNDEF(new_var);
        }
        std::string op = watchpoint.operation;
        zval *old_var = &watchpoint.old_var;

        if (watchpoint.type == yasd::WatchPointElement::VARIABLE_CHANGE) {
            zval *old_var = &watchpoint.old_var;

            if (!yasd::util::is_variable_equal(new_var, old_var)) {
                watchpoint.old_var = *new_var;
                return true;
            }
        } else {
            if (op == "<") {
                if (yasd::util::is_variable_smaller(new_var, old_var)) {
                    var_watchpoint->second->erase(watchpointIter);
                    return true;
                }
            } else if (op == ">") {
                if (yasd::util::is_variable_greater(new_var, old_var)) {
                    var_watchpoint->second->erase(watchpointIter);
                    return true;
                }
            } else if (op == "==") {
                if (yasd::util::is_variable_equal(new_var, old_var)) {
                    var_watchpoint->second->erase(watchpointIter);
                    return true;
                }
            }
        }
    }

    return false;
}

bool util::is_integer(const std::string &s) {
    if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;

    char *p;
    strtol(s.c_str(), &p, 10);

    return (*p == 0);
}

bool util::eval(char *str, zval *retval_ptr, char *string_name) {
    int origin_error_reporting = EG(error_reporting);

    // we need to turn off the warning if the variable is UNDEF
    EG(error_reporting) = 0;

    int ret;
    ret = zend_eval_string(str, retval_ptr, const_cast<char *>("yasd://debug-eval"));
    if (ret == FAILURE) {
        return false;
    }

    EG(error_reporting) = origin_error_reporting;

    return true;
}

zend_array *util::get_properties(zval *zobj) {
#if PHP_VERSION_ID >= 70400
    return zend_get_properties_for(zobj, ZEND_PROP_PURPOSE_VAR_EXPORT);
#else
    if (Z_OBJ_HANDLER_P(zobj, get_properties)) {
        return Z_OBJPROP_P(zobj);
    }
#endif
    return nullptr;
}

std::string util::get_property_name(zend_string *property_name) {
    const char *class_name, *_property_name;
    size_t _property_name_len;

    zend_unmangle_property_name_ex(property_name, &class_name, &_property_name, &_property_name_len);

    return std::string(_property_name, _property_name_len);
}

std::string util::get_option_value(const std::vector<std::string> &options, std::string option) {
    auto iter = options.begin();

    for (; iter != options.end(); iter++) {
        if (option == *iter) {
            break;
        }
    }

    return *(++iter);
}

// TODO(codinghuang): maybe we can use re2c and yacc later
zval *util::fetch_zval_by_fullname(std::string fullname) {
    int state = 0;
    char quotechar = 0;
    const char *ptr = fullname.c_str();
    const char *end = fullname.c_str() + fullname.length() - 1;

    // aa->bb->cc
    // aa[bb][cc]
    // k[0]

    struct NextZvalInfo {
        enum NextZvalType {
            ARRAY_INDEX_ASSOC,
            ARRAY_INDEX_NUM,
        };

        std::string name;
        NextZvalType type;
        const char *keyword;
        const char *keyword_end = nullptr;
        zend_array *symbol_table;
        zval *retval_ptr = nullptr;
    };

    NextZvalInfo next_zval_info;
    next_zval_info.symbol_table = zend_rebuild_symbol_table();
    next_zval_info.keyword = ptr;

    auto fetch_next_zval = [](NextZvalInfo *next_zval_info) {
        std::string name(next_zval_info->keyword, next_zval_info->keyword_end - next_zval_info->keyword);

        if (!next_zval_info->retval_ptr && name == "GLOBALS") {
            next_zval_info->symbol_table = &EG(symbol_table);
            next_zval_info->retval_ptr = &global->globals;
            return;
        } else if (next_zval_info->retval_ptr) {
            next_zval_info->symbol_table = Z_ARRVAL_P(next_zval_info->retval_ptr);
        }

        if (!next_zval_info->retval_ptr) {
            next_zval_info->retval_ptr = find_variable(next_zval_info->symbol_table, name);
        } else if (Z_TYPE_P(next_zval_info->retval_ptr) == IS_ARRAY) {
            if (next_zval_info->type == NextZvalInfo::NextZvalType::ARRAY_INDEX_NUM) {
                next_zval_info->retval_ptr =
                    find_variable(next_zval_info->symbol_table, strtoull(name.c_str(), NULL, 10));
            } else {
                name = yasd::util::stripslashes(name);
                next_zval_info->retval_ptr = find_variable(next_zval_info->symbol_table, name);
            }
        } else {
            next_zval_info->retval_ptr = yasd_zend_read_property(
                Z_OBJCE_P(next_zval_info->retval_ptr), next_zval_info->retval_ptr, name.c_str(), name.length(), 1);
        }
        next_zval_info->keyword = nullptr;
    };

    do {
        switch (state) {
        case 0:
            next_zval_info.keyword = ptr;
            state = 1;
        case 1:
            if (*ptr == '[') {
                next_zval_info.keyword_end = ptr;
                if (next_zval_info.keyword) {
                    fetch_next_zval(&next_zval_info);
                }
                state = 3;
            } else if (*ptr == '-') {
                next_zval_info.keyword_end = ptr;
                if (next_zval_info.keyword) {
                    fetch_next_zval(&next_zval_info);
                }
                state = 2;
            }
            break;
        case 2:
            assert(*ptr == '>');
            next_zval_info.keyword = ptr + 1;
            state = 1;
            break;
        case 3:
            if ((*ptr == '\'' || *ptr == '"')) {
                state = 4;
                quotechar = *ptr;
                next_zval_info.keyword = ptr + 1;
                next_zval_info.type = NextZvalInfo::NextZvalType::ARRAY_INDEX_ASSOC;
            } else if (*ptr >= '0' && *ptr <= '9') {
                state = 6;
                next_zval_info.keyword = ptr;
                next_zval_info.type = NextZvalInfo::NextZvalType::ARRAY_INDEX_NUM;
            }
            break;
        case 4:
            if (*ptr == '\\') {  // for example classname key array (Bar\\Foo::class)
                state = 10;
            } else if (*ptr == quotechar) {
                quotechar = 0;
                state = 5;
                next_zval_info.keyword_end = ptr;
                fetch_next_zval(&next_zval_info);
            }
            break;
        case 5:
            if (*ptr == ']') {
                state = 1;
            }
            break;
        case 6:
            if (*ptr == ']') {
                next_zval_info.keyword_end = ptr;
                if (next_zval_info.keyword) {
                    fetch_next_zval(&next_zval_info);
                }
                state = 1;
            }
            break;
        case 10: /* escaped character */
            state = 4;
            break;
        default:
            break;
        }
        ptr++;
    } while (ptr <= end);

    if (next_zval_info.keyword) {
        next_zval_info.keyword_end = ptr;

        fetch_next_zval(&next_zval_info);
    }

    return next_zval_info.retval_ptr;
}

std::string util::stripslashes(std::string str) {
    zend_string *tmp_zstr = zend_string_init(str.c_str(), str.length(), 0);
    php_stripslashes(tmp_zstr);
    str = std::string(ZSTR_VAL(tmp_zstr), ZSTR_LEN(tmp_zstr));
    zend_string_release(tmp_zstr);
    return str;
}

std::string util::stripcslashes(std::string str) {
    zend_string *tmp_zstr = zend_string_init(str.c_str(), str.length(), 0);
    php_stripcslashes(tmp_zstr);
    str = std::string(ZSTR_VAL(tmp_zstr), ZSTR_LEN(tmp_zstr));
    zend_string_release(tmp_zstr);
    return str;
}

std::string util::addslashes(std::string str) {
    zend_string *tmp_zstr = zend_string_init(str.c_str(), str.length(), 0);

# if PHP_VERSION_ID >= 70300
    tmp_zstr = php_addslashes(tmp_zstr);
# else
    tmp_zstr = php_addslashes(tmp_zstr, 0);
# endif
    str = std::string(ZSTR_VAL(tmp_zstr), ZSTR_LEN(tmp_zstr));
    zend_string_release(tmp_zstr);
    return str;
}

long util::microtime() {
    struct timeval t;
    gettimeofday(&t, nullptr);
    return t.tv_sec * 1000 + t.tv_usec / 1000;
}

}  // namespace yasd
