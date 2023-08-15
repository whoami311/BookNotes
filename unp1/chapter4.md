# chapter4

1. AF_前缀表示地址族，PF_前缀表示协议族。更常用AF_常值。
2. 调用connect前不必非得调用bind，因为如果需要的话，内核会确定源IP地址，并选择一个临时端口作为源端口。
3. TCP连接中connect出错情况：(1)若TCP客户没有收到SYN分节的响应，则返回ETIMEDOUT错误。(2)若对客户的SYN的响应是RST（表示复位），则表明该服务器主机在我们指定的端口上没有进程在等待与之连接。这是一种硬错误，客户一接收到RST就马上返回ECONNREFUSED错误。(3)若客户发出的SYN在中间的某个路由器上引发了一个“destination unreachable”（目的地不可达）ICMP错误，则认为是一种软错误。
4. bind捆绑操作涉及三个对象：套接字、地址和端口。其中套接字是捆绑的主体，地址和端口是捆绑在套接字上的客体。如果指定端口号为0，那么内核就在bind被调用时选择一个临时端口。如果指定IP地址为通配地址，那么内核将等到套接字已连接（TCP）或已在套接字上发出数据报（UDP）时才选择一个本地IP地址。对于IPv4来说，通配地址由常值INADDR_ANY来制定，其值一般为0。它告知内核去选择IP地址。系统预先分配in6addr_any变量并将其初始化为常值IN6ADDR_ANY_INIT。头文件<netinet/in.h>中含有in6addr_any的extern声明。

```c
// ipv4
struct sockaddr_in servaddr;
servaddr.sin_addr.s_addr = htonl(INADDR_ANY);   // wildcard
// ipv6
struct sockaddr_in6 serv;
serv.sin6_addr = in6addr_any;   // wildcard
```

5. bind函数返回的一个常见错误是EADDRINUSE（“Address already in use”，地址已使用）。

6. 当socket函数创建一个套接字时，它被假设为一个主动套接字，也就是说，它是一个将调用connect发起连接的客户套接字。listen函数把一个未连接的套接字转换成一个被动套接字，指示内核应接受指向该套接字的连接请求。调用listen导制套接字从CLOSED状态转换到LISTEN状态。
7. listen的第二个参数backlog规定了内核应该为相应套接字排队的最大连接个数。内核为任何一个给定的监听套接字维护两个队列：未完成连接队列和已完成连接队列。不要把backlog定义为0，因为不同的实现对此有不同的解释。
8. 当一个客户SYN到达时，若这些队列是满的，TCP就忽略该分节，也就是不发送RST。Linux 2.4.7系统中backlog值 0~14 对应实际已排队连接的最大数目是 3~17。
9. accept函数的addrlen是值-结果参数：调用前，我们将由*addrlen所引用的整数值置为由cliaddr所指的套接字地址结构的长度，返回时，该整数值即为由内核存放在该套接字地址结构内的确切字节数。
10. root权限才能使用1024以下的保留端口。
11. fork的两个典型用法：(1)一个进程创建一个自身的副本，这样每个副本都可以在另一个副本执行其他任务的同时处理各自的某个操作。(2)一个进程想要执行另一个程序。
12. exec函数把当前进程映像替换成新的程序文件，而且该新程序通常从main函数开始执行。进程ID并不改变。我们称调用exec的进程为调用进程(calling process)，称新执行的程序为新程序（new program）。
13. 每个文件或套接字都有一个引用计数，它是当前打开着的引用该文件或套接字的描述符的个数。真正的清理和资源释放要等到其引用计数到达0时才发生。
14. getsockname和getpeername:
- 在一个没有调用bind的TCP客户上，connect返回成功后，getsockname用于返回由内核赋予该连接的本地IP地址和本地端口号。
- 在以端口号0调用bind后，getsockname用于返回由内核赋予的本地端口号。
- getsockname可用于获取某个套接字的地址族。
- 在一个以通配IP地址调用bind的TCP服务器上，与某个客户的连接一旦建立（accept成功返回），getsockname可以用于返回由内核赋予该链接的本地IP地址。套接字描述符参数必须是已连接套接字的描述符，而不是监听套接字的描述符。
- 当一个服务器是由调用过accept的某个进程通过调用exec执行程序时，它能够获取客户身份的唯一途径便是调用getpeername。