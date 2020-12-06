English | [中文](./README-CN.md)

# yasd

Yet Another Swoole Debugger.

## How to use it

### build

```bash
phpize --clean && \
phpize && \
./configure && \
make clean && \
make && \
make install
```

set the ini file:

```ini
zend_extension=yasd.so
```

see the extension info:

```bash
php --ri yasd
```

All command supports fuzzy matching of priorities.

### start debug

```bash
php -e test.php
```

### list

```bash
l
```

list source code

### breakpoint

```bash
b absolute-file-path lineno
```

This will also save the breakpoint information in the cache file -- `.breakpoints_file.log`

You can change this file name by the ini file, e.g.

```ini
yasd.breakpoints_file="yasd.log"
```

If the cache file exists, the breakpoints in the file are automatically loaded when debug is started.

### Delete breakpoints

```bash
d absolute-file-path lineno
```

If a breakpoint is set or deleted without specifying `absolute-file-path`, It will set breakpoint in the current file.

### watch point

We can monitor the variable changes or conditions, for example:

```cpp
watch a
```

```cpp
watch a > 10
```

```cpp
watch name == swoole
```

### run

```bash
r
```

### step over

```bash
n
```

### step into

```bash
s
```

### step out

```bash
f
```

### backtrace

```bash
bt
```

### info

show all breakpoints:

```bash
i
```

### continue

```bash
c
```

### quit

```bash
q
```

### print

```bash
p
```

variable name do not need to have `$`, for example:

```bash
p a
p this
p this->prop
```

### level

```bash
le
```
