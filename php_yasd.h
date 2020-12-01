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

#ifndef PHP_YASD_H_
#define PHP_YASD_H_

#include "php/main/php.h"

extern zend_module_entry yasd_module_entry;
#define phpext_yasd_ptr &yasd_module_entry

#define PHP_YASD_VERSION "0.1.0"

namespace yasd { namespace function {
class ReturnValue {
  public:
    zval value;
    ReturnValue() {
        value = {};
    }
    ~ReturnValue() {
        zval_dtor(&value);
    }
};

ReturnValue call(const std::string &func_name, int argc, zval *argv);
}
}

#endif /* PHP_YASD_H_ */
