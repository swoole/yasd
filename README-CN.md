[English](./README.md) | 中文

# yasd

另一个 Swoole 调试器。

## 如何使用它

### 安装

```bash
git submodule update --init && \
phpize --clean && \
phpize && \
./configure && \
make clean && \
make && \
make install
```

设置 php.ini 文件：

```ini
zend_extension=yasd.so
```

查看扩展信息：

```bash
php --ri yasd
```

所有命令都支持优先级模糊匹配。

如`li`、`lis`、`list`都等于`l`，查看源码

### 开始调试 run

```bash
php -e test.php
```

### 查看源码 list

```bash
l
```

### 设置断点

```bash
b 文件的绝对路径 需要断点的行号
```

默认会将断点信息保存在缓存文件 `.breakpoints_file.log` 中；

你也可以通过修改 php.ini 指定此文件名，如：

```ini
yasd.breakpoints_file=yasd.log
```

如果缓存文件存在，当启动调试时，文件中的断点信息会被自动加载；

### 删除断点 delete

```bash
d 文件的绝对路径 断点所在的行号
```

如果设置或者删除断点时，不指定文件绝对路径，默认是当前停留的文件。

### 监视点 watch

我们可以监控变量变化或条件，例如：

```cpp
watch a
```

```cpp
watch a > 10
```

```cpp
watch name == swoole
```

### 运行 run

```bash
r
```

### 下一步 step over

```bash
n
```

遇到函数的时候，不会进入函数内部

### 下一步 step into

```bash
s
```

遇到函数的时候，会进入函数内部

### 跳出当前函数 finish

```bash
f
```

### 查看调用栈

```bash
bt
```

### 查看所有断点信息 info

```bash
i
```

### 继续运行 continue

```bash
c
```

### 退出 quit

```bash
q
```

### 打印变量 print

```bash
p
```

变量名字不需要带`$`，例如:

```bash
p a
p this
p this->prop
```

### 查看当前所在的协程 level

```bash
le
```
