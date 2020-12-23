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

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <string>
#include <cstddef>
#include <algorithm>

#define YASD_DEFAULT_ALIGNMENT sizeof(size_t)
#define YASD_MEM_ALIGNED_SIZE(size) YASD_MEM_ALIGNED_SIZE_EX(size, YASD_DEFAULT_ALIGNMENT)
#define YASD_MEM_ALIGNED_SIZE_EX(size, alignment) (((size) + ((alignment) -1LL)) & ~((alignment) -1LL))

namespace yasd {
class Buffer {
 private:
    void alloc(size_t _size) {
        _size = YASD_MEM_ALIGNED_SIZE(_size);
        length = 0;
        size = _size;
        offset = 0;
        str = reinterpret_cast<char *>(malloc(_size));
    }

    void move(Buffer &&src) {
        str = src.str;
        length = src.length;
        offset = src.offset;
        size = src.size;

        src.str = nullptr;
        src.length = 0;
        src.size = 0;
        src.offset = 0;
    }

 public:
    size_t length;
    size_t size;
    off_t offset;
    char *str;

    Buffer() {
        length = size = offset = 0;
        str = nullptr;
    }

    explicit Buffer(size_t _size) {
        alloc(_size);
    }

    Buffer(const char *_str, size_t _length) {
        alloc(_length);
        memcpy(str, _str, _length);
        length = _length;
    }

    Buffer(Buffer &_str) {
        alloc(_str.size);
        memcpy(_str.str, str, _str.length);
        length = _str.length;
        offset = _str.offset;
    }

    Buffer(Buffer &&src) {
        move(std::move(src));
    }

    Buffer &operator=(Buffer &src) {
        if (&src == this) {
            return *this;
        }
        if (str) {
            free(str);
        }
        alloc(src.size);
        memcpy(src.str, str, src.length);
        length = src.length;
        offset = src.offset;
        return *this;
    }

    Buffer &operator=(Buffer &&src) {
        if (&src == this) {
            return *this;
        }
        if (str) {
            free(str);
        }
        move(std::move(src));
        return *this;
    }

    ~Buffer() {
        if (str) {
            free(str);
        }
    }

    inline char *value() {
        return str;
    }

    inline size_t get_length() {
        return length;
    }

    inline size_t capacity() {
        return size;
    }

    inline std::string to_std_string() {
        return std::string(str, length);
    }

    inline bool grow(size_t incr_value) {
        length += incr_value;
        if (length == size && !reserve(size * 2)) {
            return false;
        } else {
            return true;
        }
    }

    Buffer *substr(size_t offset, size_t len) {
        if (offset + len > length) {
            return nullptr;
        }
        auto _substr = new Buffer(len);
        _substr->append(str + offset, len);
        return _substr;
    }

    bool empty() {
        return str == nullptr || length == 0;
    }

    inline void clear() {
        length = 0;
        offset = 0;
    }

    inline bool extend() {
        return extend(size * 2);
    }

    inline bool extend(size_t new_size) {
        assert(new_size > size);
        return reserve(new_size);
    }

    inline bool extend_align(size_t _new_size) {
        size_t align_size = YASD_MEM_ALIGNED_SIZE(size * 2);
        while (align_size < _new_size) {
            align_size *= 2;
        }
        return reserve(align_size);
    }

    bool reserve(size_t new_size);
    bool repeat(const char *data, size_t len, size_t n);
    bool append(const char *append_str, size_t length);

    inline bool append(const std::string &append_str) {
        return append(append_str.c_str(), append_str.length());
    }

    inline bool append(const Buffer &append_str) {
        size_t new_size = length + append_str.length;
        if (new_size > size) {
            if (!reserve(new_size)) {
                return false;
            }
        }

        memcpy(str + length, append_str.str, append_str.length);
        length += append_str.length;
        return true;
    }

    void print();

    char *pop(size_t init_size);
    void reduce(off_t offset);
};

inline Buffer *make_string(size_t size) {
    return new Buffer(size);
}

}  // namespace yasd
