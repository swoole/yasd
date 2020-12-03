[English](./README.md) | 中文

# yasd

另一个 Swoole 调试器。

## 如何使用它

### 安装

```bash
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
b absolute-file-path lineno
```

这也会把断点信息保存在缓存文件 `.breakpoints_file.bp` 中

如果缓存文件存在，当启动调试时，文件中的断点信息会被自动加载

### 删除断点 delete

```bash
d absolute-file-path lineno
```

### 运行 run

```bash
r
```

### 下一步 next

```bash
n
```

### 进入 step

```bash
s
```

### 进入后退出 finish

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

### 查看所在当前协程 level

```bash
le
```
