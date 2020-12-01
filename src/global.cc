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
#include "include/global.h"

yasd::Global *global;

zend_function *get_cid_function = nullptr;

int64_t get_cid() {
    zval retval;

    if (!get_cid_function) {
        return 0;
    }

    get_cid_function->internal_function.handler(nullptr, &retval);

    return Z_LVAL(retval) == -1 ? 0 : Z_LVAL(retval);
}

namespace yasd {
Global::Global(/* args */) {
    breakpoints = new std::map<BREAKPOINT>();
    contexts = new std::map<int, Context *>();
}

Global::~Global() {
    delete contexts;
    contexts = nullptr;
    delete breakpoints;
    breakpoints = nullptr;
}

Context *Global::get_current_context() {
    yasd::Context *context = nullptr;
    int64_t cid = get_cid();

    auto iter = global->contexts->find(cid);

    if (iter != global->contexts->end()) {
        context = iter->second;
    } else {
        context = new yasd::Context();
        context->cid = cid;
        global->contexts->insert(std::make_pair(cid, context));
    }

    return context;
}
}  // namespace yasd
