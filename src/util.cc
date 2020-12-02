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
#include "include/util.h"
#include "include/common.h"
#include "include/global.h"

BEGIN_EXTERN_C()
#include "php/ext/standard/php_var.h"
END_EXTERN_C()

#include "./php_yasd.h"

#include <iostream>

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
            arr.push_back(target.substr(k, i - k));
            i += del_len;
            k = i;
        } else {
            i++;
        }
    }

    arr.push_back(target.substr(k, i - k));
    return arr;
}

HashTable *Util::get_defined_vars() {
    HashTable *symbol_table;

    symbol_table = zend_rebuild_symbol_table();

    if (EXPECTED(symbol_table != nullptr)) {
        symbol_table = zend_array_dup(symbol_table);
    }

    return symbol_table;
}

void Util::print_var(std::string var_name) {
    zval *var;
    HashTable *defined_vars;
    zend_execute_data *execute_data = EG(current_execute_data);

    // print $this
    if (var_name == "this") {
        php_var_dump(ZEND_THIS, 1);
        return;
    }

    // print property
    auto exploded_var_name = yasd::Util::explode(var_name, "->");
    if (exploded_var_name.size() == 2) {
        zval *property = yasd_zend_read_property(
            Z_OBJCE_P(ZEND_THIS), ZEND_THIS, exploded_var_name[1].c_str(), exploded_var_name[1].length(), 0);
        php_var_dump(property, 1);
        return;
    }

    defined_vars = get_defined_vars();

    var = zend_hash_str_find(defined_vars, var_name.c_str(), var_name.length());
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

        yasd::Util::printf_info(yasd::Color::YASD_ECHO_GREEN, "reload breakpoint at %s:%d", filename.c_str(), lineno);

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
    return std::string(YASD_G(breakpoints_file)) + ".bp";
}

void Util::cache_breakpoint(std::string filename, int lineno) {
    std::ofstream file;
    std::string path = get_breakpoint_cache_filename();

    file.open(path, std::ios_base::app);
    file << filename + ":" + std::to_string(lineno) + "\n";
    file.close();
}

}  // namespace yasd
