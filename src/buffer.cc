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

#include "include/buffer.h"

#define YASD_MAX(A, B) ((A) > (B) ? (A) : (B))
#define YASD_MIN(A, B) ((A) < (B) ? (A) : (B))

namespace yasd {
char *Buffer::pop(size_t init_size) {
    assert(length >= (size_t) offset);

    char *val = str;
    size_t _length = length - offset;
    size_t alloc_size = YASD_MEM_ALIGNED_SIZE(_length == 0 ? init_size : YASD_MAX(_length, init_size));

    char *new_val = reinterpret_cast<char *>(malloc(alloc_size));
    if (new_val == nullptr) {
        return nullptr;
    }

    str = new_val;
    size = alloc_size;
    length = _length;
    if (length > 0) {
        memcpy(new_val, val + offset, length);
    }
    offset = 0;

    return val;
}

/**
 * migrate data to head, [offset, length - offset] -> [0, length - offset]
 */
void Buffer::reduce(off_t _offset) {
    assert(_offset >= 0 && (size_t) _offset <= length);
    if (_offset == 0) {
        return;
    }
    length -= _offset;
    offset = 0;
    if (length == 0) {
        return;
    }
    memmove(str, str + _offset, length);
}

void Buffer::print() {
    printf("Buffer[length=%zu,size=%zu,offset=%jd]=%.*s\n",
           length,
           size,
           (intmax_t) offset,
           static_cast<int>(length),
           str);
}

bool Buffer::append(const char *append_str, size_t _length) {
    size_t new_size = length + _length;
    if (new_size > size && !reserve(new_size)) {
        return false;
    }

    memcpy(str + length, append_str, _length);
    length += _length;
    return true;
}

bool Buffer::reserve(size_t new_size) {
    if (size == 0) {
        alloc(new_size);
        return true;
    }

    new_size = YASD_MEM_ALIGNED_SIZE(new_size);
    char *new_str = reinterpret_cast<char *>(realloc(str, new_size));

    str = new_str;
    size = new_size;

    return true;
}

bool Buffer::repeat(const char *data, size_t len, size_t n) {
    if (n <= 0) {
        return false;
    }
    if (len == 1) {
        if ((size < length + n) && !reserve(length + n)) {
            return false;
        }
        memset(str + length, data[0], n);
        length += n;

        return true;
    }
    for (size_t i = 0; i < n; i++) {
        append(data, len);
    }
    return true;
}

}  // namespace yasd
