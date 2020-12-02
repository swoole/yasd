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

### breakpoint

```bash
b absolute-file-path lineno
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
t
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
