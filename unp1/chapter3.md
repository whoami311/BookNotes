# chapter3

1. IPv4地址和TCP或UDP端口号在套接字地址结构中总是以网络字节序来存储。sin_zero字段未曾使用，不过在填写这种套接字地址结构时，我们总是把该字段置为0。

2. 当作为一个参数传递进任何套接字函数时，套接字地址结构总是以引用形式来传递。然而以这样的指针作为参数之一的任何套接字函数必须处理来自所支持的任何协议族的套接字地址结构。这就要求对这些函数的任何调用都必须要将指向特定于协议的套接字地址结构的指针进行类型强制转换（casting），变成指向某个通用套接字地址结构的指针。

```c
struct sockaddr_in serv;
// fill in serv{}
bind(sockfd, (struct sockaddr*)&serv, sizeof(serv));
```

3. 作为IPv6套接字API的一部分而定义的新的通用套接字地址结构克服了现有struct sockaddr的一些缺点。新的sockaddr_storage足以容纳系统所支持的任何套接字地址结构。在<netinet/in.h>中定义。两个特点：(1)sockaddr_storage能够满足最苛刻的对其要求。(2)sockaddr_storage足够大，能够容纳系统支持的任何套接字地址结构。

4. 小端字节序：将低序字节存储在起始地址。大端字节序：将高序字节存储在起始地址。网际协议使用大端字节序来传送这些多字节参数。

5. 字节序转换函数

```c
#include <netinet/in.h>

uint16_t htons(uint16_t host16bitvalue);
uint32_t htonl(uint32_t host32bitvalue);

uint16_t ntohs(uint16_t net16bitvalue);
uint16_t ntohl(uint32_t net32bitvalue);
```

h代表host，n代表network，s代表short，l代表long。应该把s视为一个16位的值，把l视为一个32位的值。

6. 字节操纵函数：bzero、bcopy、bcmp，内存操纵函数：memset、memcpy、memcmp。记住memcpy两个指针参数顺序的方法之一是记着它们是按照与C中的赋值语句相同的顺序从左到右书写的：dest = src;。所有ANSI C的memXXX函数都需要一个长度参数，而且它总是最后一个参数。

7. 地址转换函数：inet_aton一个特征：如果addrptr指针为空，那么该函数仍然对输入的字符串执行有效性检查，但是不存储任何结果。

8. inet_addr（已被废弃）存在问题：（1）所有2^32个可能的二进制值都是有效的IP地址（从0.0.0.0到255.255.255.255），但是当出错时该函数返回INADDR_NONE常值（通常是一个32位均为1的值）。这意味着点分十进制数串255.255.255.255（这是IPv4的有限广播地址）不能由该函数处理。因为它的二进制值被用来指示该函数失败。（2）一些手册页面声明该函数出错时返回-1而不是INADDR_NONE。这样在对该函数的返回值（一个无符号的值）和一个负常值（-1）进行比较时可能会发生问题，具体取决于C编译器。
9. inet_ntoa将一个32位的网络字节序二进制IPv4地址转换成相应的点分十进制数串。由该函数的返回值所指向的字符串驻留在静态内存中。这意味着该函数是不可重入的。
10. inet_pton和inet_ntop对IPv4地址和IPv6地址都适用。p和n分别代表表达（presentation）和数值（numeric）。如果以不被支持的地址族作为family参数，这两个函数就都返回一个错误，并将errno置为EAFNOSUPPORT。inet_ntop的len如果太小，不足以容纳表达格式结果，那么返回一个空指针，并将errno置为ENOSPC。
11. 字节流套接字上的read和write函数所表现的行为不同于通常的文件I/O。字节流套接字上调用read或write输入或输出的字节数可能比请求的数量少，原因在于内核中用于套接字的缓冲区可能已达到了极限。这个现象在读一个字节流套接字时很常见，但是在写一个字节流套接字时只能在该套接字为非阻塞的前提下才会出现。