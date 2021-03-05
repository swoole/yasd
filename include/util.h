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

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <utility>

#include "main/php.h"

namespace yasd {
class Util {
 public:
    static HashTable *get_defined_vars();
    static zval *find_variable(std::string var_name);
    static zval *find_variable(zend_array *symbol_table, zend_ulong index);
    static zval *find_variable(zend_array *symbol_table, std::string var_name);
    static void print_var(std::string fullname);
    static void print_property(std::string obj_name, std::string property_name);

    static void printf_info(int color, const char *format, ...);

    template <typename... Args>
    static void printfln_info(int color, const char *format, Args... args) {
        printf_info(color, format, args...);
        std::cout << std::endl;
    }

    static void show_breakpoint_hit_info();
    static const char *get_executed_filename();
    static const char *get_executed_function_name();
    static int get_executed_file_lineno();
    static const char *get_prev_executed_filename();
    static const char *get_prev_executed_function_name();
    static int get_prev_executed_file_lineno();
    static bool is_match(std::string sub_str, std::string target_str);

    static void clear_breakpoint_cache_file();
    static std::string get_breakpoint_cache_filename();
    static void cache_breakpoint(std::string filename, int lineno);

    static bool is_variable_equal(zval *op1, zval *op2);
    static bool is_variable_smaller(zval *op1, zval *op2);
    static bool is_variable_greater(zval *op1, zval *op2);

    static bool is_hit_watch_point();

    static bool is_integer(const std::string &s);

    static bool eval(char *str, zval *retval_ptr, char *string_name);

    static zend_array *get_properties(zval *zobj);

    // get the property name of a common property, including public, protected, private
    static std::string get_property_name(zend_string *property_name);

    static std::string get_option_value(const std::vector<std::string> &options, std::string option);

    static zval *fetch_zval_by_fullname(std::string fullname);

    static std::string stripslashes(std::string str);
    static std::string stripcslashes(std::string str);
    static std::string addslashes(std::string str);

    static long microtime(void);
};
}  // namespace yasd
