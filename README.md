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

### list source

```bash
l
```

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

### level

```bash
le
```
