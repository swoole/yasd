#include "php_yasd_cxx.h"

namespace zend { namespace function {
    bool call(zend_fcall_info_cache *fci_cache, uint32_t argc, zval *argv, zval *retval) {
        bool success;
        success = yasd_zend_call_function_ex(nullptr, fci_cache, argc, argv, retval) == SUCCESS;

        if (UNEXPECTED(EG(exception))) {
            zend_exception_error(EG(exception), E_ERROR);
        }
        return success;
    }
}  // namespace function
}  // namespace zend
