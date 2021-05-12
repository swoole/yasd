English | [中文](./README-CN.md)

# yasd

Yet Another Swoole Debugger. [document](https://huanghantao.github.io/yasd-wiki/)

## How to use it

### build

You'll need to install the Boost library first.

macOS:

```bash
brew install boost
```

Ubuntu:

```bash
apt-get install libboost-all-dev
```

CentOS:

```bash
yum install boost boost-devel
```

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
zend_extension=yasd
```

see the extension info:

```bash
php --ri yasd
```

### IDE debug mode

```ini
zend_extension=yasd
yasd.debug_mode=remote
yasd.remote_host=127.0.0.1
yasd.remote_port=9000
```

### cmd debug mode

All command supports fuzzy matching of priorities.

set the ini file:

```ini
zend_extension=yasd
yasd.debug_mode=cmd
```

#### start debug

```bash
php -e test.php
```

#### list

```bash
l
```

list source code

#### breakpoint

```bash
b absolute-file-path lineno
```

This will also save the breakpoint information in the cache file -- `.breakpoints_file.log`

You can change this file name by the ini file, e.g.

```ini
yasd.breakpoints_file="yasd.log"
```

If the cache file exists, the breakpoints in the file are automatically loaded when debug is started.

#### Delete breakpoints

```bash
d number
```

If a breakpoint is set or deleted without specifying `absolute-file-path`, It will set breakpoint in the current file.

#### watch point

We can monitor the variable changes or expression, for example:

```cpp
watch $a
```

```cpp
watch $a > 10
```

```cpp
watch $name == 'swoole'
```

#### unwatch point

```cpp
unwatch number
```

#### run

```bash
r
```

#### step over

```bash
n
```

#### step into

```bash
s
```

#### step out

```bash
f
```

#### backtrace

```bash
bt
```

#### info

show all breakpoints:

```bash
info breakpoints
```

show all watchpoints:

```bash
info watchpoints
```

#### continue

```bash
c
```

#### quit

```bash
q
```

#### print

```bash
p $a
p $this
p $this->prop
```

#### level

```bash
le
```

### Q&A

#### Debugging with phpstorm (remote mode)

- the port phpstorm listens on should be the one used for xdebug, also make sure remote connections are allowed:
![image](https://user-images.githubusercontent.com/21345604/109924706-c2044180-7cd1-11eb-80a1-79af023ea01e.png)
- do **NOT** forget to use `-e` option when running scripts, debugging will not work otherwise

- ensure that phpstorm listens on incoming connections:

![image](https://user-images.githubusercontent.com/21345604/109924880-042d8300-7cd2-11eb-9972-002806afbb6c.png)

#### Slow Start Framework

When using `yasd`, if the framework starts slowly (most of the time it's because the framework is scanning a lot of files), you can execute the following command.

##### Hyperf

```bash
composer dump-autoload -o
```

Then modify the following configuration `config/config.php`:

```php
'scan_cacheable' => env('SCAN_CACHEABLE', true)
```

##### Imi

We did not find a slow start problem.

##### EasySwoole

We did not find a slow start problem.

##### MixPHP

We did not find a slow start problem.

##### Swoft

We did not find a slow start problem.

##### One

We did not find a slow start problem.

#### The breakpoint is not triggered

1. No absolute path to the file is used
2. The breakpoint is not a valid line, such as a function declaration, blank line, etc
3. The code is generated with proxy classes, such as Hyperf. So you need to set breakpoints in the proxy class.
