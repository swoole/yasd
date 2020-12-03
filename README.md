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
li
```

### breakpoint

```bash
b absolute-file-path lineno
```

This will also save the breakpoint information in the cache file -- `.breakpoints_file.bp`

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
l
```
