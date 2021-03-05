#pragma once

#include "php_yasd.h"

#include <string>

namespace zend { namespace function {
bool call(zend_fcall_info_cache *fci_cache, uint32_t argc, zval *argv, zval *retval);
} // namespace function
}
