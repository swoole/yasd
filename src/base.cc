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
#include "include/context.h"
#include "include/global.h"
#include "include/cmder.h"

#include <iostream>

static void (*old_execute_ex)(zend_execute_data *execute_data);

yasd::StackFrame *save_prev_stack_frame(zend_execute_data *execute_data) {
    yasd::Context *context = global->get_current_context();

    if (!EG(current_execute_data)->prev_execute_data) {
        return nullptr;
    }

    yasd::StackFrame *frame = new yasd::StackFrame();
    frame->level = context->level;
    frame->filename = yasd::Util::get_prev_executed_filename();
    frame->lineno = yasd::Util::get_prev_executed_file_lineno();
    frame->function_name =
        execute_data->func->common.function_name ? execute_data->func->common.function_name->val : "main";
    context->strace->push_back(frame);
    return frame;
}

void drop_prev_stack_frame(yasd::StackFrame *frame) {
    yasd::Context *context = global->get_current_context();

    context->strace->pop_back();
    delete frame;
}

void yasd_execute_ex(zend_execute_data *execute_data) {
    yasd::Context *context = global->get_current_context();

    context->level++;
    yasd::StackFrame *frame = save_prev_stack_frame(execute_data);
    old_execute_ex(execute_data);
    drop_prev_stack_frame(frame);
    context->level--;
}

void register_get_cid_function() {
    if (zend_hash_str_find_ptr(&module_registry, ZEND_STRL("swoole"))) {
        zend_string *classname = zend_string_init(ZEND_STRL("Swoole\\Coroutine"), 0);
        zend_class_entry *class_handle = zend_lookup_class(classname);
        zend_string_release(classname);

        get_cid_function = reinterpret_cast<zend_function *>(
            zend_hash_str_find_ptr(&(class_handle->function_table), ZEND_STRL("getcid")));
    }
}

void yasd_rinit(int module_number) {
    old_execute_ex = zend_execute_ex;
    zend_execute_ex = yasd_execute_ex;

    global = new yasd::Global();
    cmder = new yasd::Cmder();
}
