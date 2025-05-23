# 目标文件里有什么

## ELF（Executable Linkable Format） 文件

| ELF 文件类型 | 说明 | 实例 |
| - | - | - |
| 可重定位文件（Relocatable File）| 这类文件包含了代码和数据，可以被用来链接成可执行文件或共享目标文件，静态链接库也可以归为这一类 | Linux 的 .o、Windows 的 .obj |
| 可执行文件（Executable File） | 这类文件包含了可以直接执行的程序，它的代表就是 ELF 可执行文件，它们一般都没有扩展名 | 比如 /bin/bash 文件、Windows 的 .exe |
| 共享目标文件（Shared Object File）| 这种文件包含了代码和数据，可以在以下两种情况下使用。一种是链接器可以使用这种文件跟其他的可重定位文件和共享目标文件链接，产生新的目标文件。第二种是动态链接器可以将几个这种共享目标文件与可执行文件结合，作为进程映像的一部分来运行 | Linux 的 .so，如 /lib/glibc-2.5.so、Windows 的 DLL |
| 核心转储文件（Core Dump File） | 当进程意外终止时，系统可以将该进程的地址空间的内容及终止时的一些其他信息转储到核心转储文件 | Linux 下的 core dump |

## 目标文件是什么样的

1. .bss 段

未初始化的全局变量和局部静态变量默认值都为 0，所以为它们在 .data 段分配空间并且存放数据 0 是没有必要的。程序运行的时候他们的确是要占内存空间的，并且可执行文件必须记录所有未初始化的全局变量和局部静态变量的大小总和，记为 .bss 段。所以 .bss 段只是为未初始化的全局变量和局部静态变量预留位置而已，它并没有内容，所以它在文件中也不占据空间。

程序源代码被编译以后主要分为两种段：程序指令和程序数据。代码段属于程序指令，而数据段和 .bss 段属于程序数据。

2. 数据和指令分段的好处：

- 当程序被装载后，数据和指令分别被映射到两个虚存区域。由于数据区域对于进程来说是可读写的，而指令区域对于进程来说是只读的，所以这两个虚存区域的权限可以被分别设置成可读写和只读。这样可以防止程序的指令被有意或无意地改写。
- 对于现代的 CPU 来说，它们有着极为强大的缓存（Cache）体系。由于缓存在现代的计算机中地位非常重要，所以程序必须尽量提高缓存的命中率。指令区和数据区的分离有利于提高程序的局部性。现在 CPU 的缓存一般都被设计成数据缓存和指令缓存分离，所以程序的指令和数据被分开存放对 CPU 的缓存命中率提高有好处。
- 当系统中运行着多个该程序的副本时，它们的指令都是一样的，所以内存中只需要保存一份该程序的指令部分。对于指令这种只读的区域来说是这样，对于其他的只读数据也一样，比如很多程序里面带有的图标、图片、文本等资源也是属于可以共享的。当然每个副本进程的数据区域是不一样的，它们是进程私有的。共享指令的概念在现代的操作系统里面占据了极为重要的地位，特别是在有动态链接的系统中，可以节省大量的内存。

## 挖掘 SinpleSection.o

真正了不起的程序员对自己的程序的每一个字节都了如指掌。

## 链接的接口——符号

符号表中的符号分类：

- 定义在本目标文件的全局符号，可以被其他目标文件引用。
- 在本目标文件中引用的全局符号，却没有定义在本目标文件，这一般叫做**外部符号（External Symbol）**。
- 段名，这种符号往往由编译器产生，它的值就是该段的起始地址。
- 局部符号，这类符号只在编译单元内部可见。调试器可以使用这些符号来分析程序或崩溃时的核心转储文件。这些局部符号对于链接过程没有作用，链接器往往也忽略他们。
- 行号信息，即目标文件指令与源代码中代码行的对应关系，它也是可选的。

### 函数签名

**函数签名（Function Signature）**：函数签名包含了一个函数的信息，包括函数名、它的参数类型、它所在的类和名称空间及其他信息。

### extern "C"

C++ 编译器会将在 `extern "C"` 的大括号内部的代码当做 C 语言代码处理。C++ 的名称修饰机制将不会起作用。

头文件声明了一些 C 语言的函数和全局变量，但是这个头文件可能会被 C 语言代码或 C++ 代码包含。

C++ 编译器会在编译 C++ 的程序时默认定义这个宏 `__cplusplus`。

```c++
#ifdef __cplusplus
extern "C" {
#endif

void *memset(void*, int, size_t);

#ifdef __cplusplus
}
#endif
```

**强引用与弱引用**

对外部目标文件的符号引用在目标文件被最终链接成可执行文件时，它们需要被正确决议，如果没有找到该符号的定义，链接器就会报符号未定义错误，这种被称为**强引用（Strong Reference）**。与之对应还有一种**弱引用（Weak Reference）**，在处理弱引用时，如果该符号有定义，则链接器将该符号的引用决议；如果该符号未被定义，则链接器对于该引用不报错。