/**
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "include/util.h"
#include "include/common.h"

BEGIN_EXTERN_C()
#include "php/ext/standard/php_var.h"
END_EXTERN_C()

#include <iostream>

namespace yasd {

std::vector<std::string> Util::explode(std::string const &s, char delim) {
    std::vector<std::string> result;
    std::istringstream iss(s);

    for (std::string token; std::getline(iss, token, delim);) {
        result.push_back(std::move(token));
    }

    return result;
}

HashTable *Util::get_defined_vars() {
    HashTable *symbol_table;

    symbol_table = zend_rebuild_symbol_table();

    if (EXPECTED(symbol_table != nullptr)) {
        symbol_table = zend_array_dup(symbol_table);
    }

    return symbol_table;
}

void Util::print_var(const char *var_name, size_t var_name_length) {
    zval *var;
    HashTable *defined_vars;

    defined_vars = get_defined_vars();

    var = zend_hash_str_find(defined_vars, var_name, var_name_length);
    if (!var) {
        std::cout << "not found variable $" << var_name << std::endl;
    } else {
        php_var_dump(var, 1);
    }
}

void Util::printf_info(int color, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(yasd_info_buf, sizeof(yasd_info_buf), format, args);
    va_end(args);
    switch (color) {
    case yasd::Color::YASD_ECHO_RED:
        std::cout << "\e[31m" << yasd_info_buf << "\e[0m" << std::endl;
        break;
    case yasd::Color::YASD_ECHO_GREEN:
        std::cout << "\e[32m" << yasd_info_buf << "\e[0m" << std::endl;
        break;
    case yasd::Color::YASD_ECHO_YELLOW:
        std::cout << "\e[33m" << yasd_info_buf << "\e[0m" << std::endl;
        break;
    case yasd::Color::YASD_ECHO_BLUE:
        std::cout << "\e[34m" << yasd_info_buf << "\e[0m" << std::endl;
        break;
    case yasd::Color::YASD_ECHO_MAGENTA:
        std::cout << "\e[35m" << yasd_info_buf << "\e[0m" << std::endl;
        break;
    case yasd::Color::YASD_ECHO_CYAN:
        std::cout << "\e[36m" << yasd_info_buf << "\e[0m" << std::endl;
        break;
    case yasd::Color::YASD_ECHO_WHITE:
        std::cout << "\e[37m" << yasd_info_buf << "\e[0m" << std::endl;
        break;
    default:
        break;
    }
}

void Util::show_breakpoint_hit_info() {
    printf_info(YASD_ECHO_GREEN, "stop at breakponit");
}

const char *Util::get_executed_filename() {
    zend_string *filename;

    filename = zend_get_executed_filename_ex();

    if (UNEXPECTED(filename == nullptr)) {
        return "";
    }

    return ZSTR_VAL(zend_string_copy(filename));
}

int Util::get_executed_file_lineno() {
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

    return "uknow file";
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
}  // namespace yasd
