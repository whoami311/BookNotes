# 第一章

1. bzero（带2个参数）比memset（带3个参数）更好记忆。

2. 计算机网络各层对等实体间交换的单位信息称为 协议数据单元（protocol data unit，PDU），分节（segment）就是对应于TCP传输层的PDU。除了最底层（物理层）外，每层的PDU通过由紧邻下层提供给本层的服务接口，作为下层的 服务数据单元（service data unit，SDU）传递给下层，并由下层间接完成本层的PDU交换。如果本层的PDU大小超过紧邻下层的最大SDU限制，那么本层还要事先把PDU划分成若干个合适的片段让下层分开载送，再在相反方向把这些片段重组成PDU。同一层内SDU作为PDU的净荷（payload）字段出现，因此可以说上层PDU由本层PDU（通过其SDU字段）承载。每层的PDU除用于承载紧邻上层的PDU（即承载数据）外，也用于承载本层协议内部通信所需的控制信息。

3. 应用层实体间（进程）交换的PDU称为应用数据（application data），其中在TCP应用进程之间交换的是没有长度限制的单个双向字节流，在UDP应用进程之间交换的是其长度不超过UDP发送缓冲区大小的单个记录（record）。传输层实体间交换的PDU称为消息（message），其中TCP的PDU特称为分节（segment）。消息或分节的长度是有限的。在TCP传输层中，发送端TCP把来自应用进程的字节流数据按顺序经分割后封装在各个分节中传送给接收端TCP，其中每个分节所封装的数据既可能是发送端应用进程单次输出操作的结果，也可能是连续数次输出操作的结果，而且每个分节所封装的单次输出操作的结果或者首尾两次输出操作的结果既可能是完整的，也可能是不完整的，具体取决于可在连接建立阶段由对端通告的 最大分节大小（maximum segment size，MSS）以及外出接口的最大传输单元（maximum transmission unit，MTU）或外出路径的路径MTU（如果网络层具有路径MTU发现功能，如IPv6）。UDP传输层相当简单，发送端UDP就把来自应用进程的单个记录整个封装在UDP消息中传送给接收端UDP。

4. 网络层实体间交换的PDU称为IP数据报（IP datagram），其长度有限：IPv4数据报最大65535字节，IPv6数据报最大65575字节。发送端IP把来自传输层的消息（或TCP分节）整个封装在IP数据报中传送。链路层实体间交换的PDU称为帧（frame），其长度取决于具体的接口。IP数据报由IP首部和所承载的传输层数据（即网络层的SDU）构成。过长的IP数据报无法封装在单个帧中，需要先对其SDU进行分片（fragmentation），再把分成的各个片段（frament）冠以新的IP首部封装到多个帧中。在一个IP数据报从源端到目的端的传送过程中，分片操作既可能发生在源端，也可能发生在途中，而其逆操作即重组（reassembly）一般只发生在目的端。TCP/IP协议族为提高效率会尽可能避免IP的分片/重组操作：TCP根据MSS和MTU限定每个分节的大小就是这个目的；另外，IPv6禁止在途中的分片操作（基于其路径MTU发现功能），IPv4也尽量避免这种操作。不论是否分片，都由IP作为链路层的SDU传入链路层，并由链路层封装在帧中的数据称为分组（packet，俗称包）。可见一个分组既可能是一个完整的IP数据报，也可能是某个IP数据报的SDU的一个片段被冠以新的IP首部后的结果。另外，这里讨论的MSS是应用层（TCP）与传输层之间的接口属性，MTU则是网络层和链路层之间的接口属性。

5. 从TCP套接字读取数据时，总是需要把read编写在某个循环中，当read返回0（表明对端关闭连接）或负值（表明发生错误）时终止循环。

6. 包裹函数命名：相比给函数名加一个“e”前缀或“_e”后缀，首字母大写是最少分散注意力的。

7. Unix errno值：只要一个Unix函数中有错误发生，全局变量errno就被置为一个指明该错误类型的正值，函数本身则通常返回-1。errno的所有正数错误值都是常值，具有以“E”开头的全大写字母名字，并通常在<sys/errno.h>头文件中定义。值0不表示任何错误。在全局变量中存放errno值对于共享所有全局变量的多个线程并不适合。

8. 调用sprintf无法检查目的缓冲区是否溢出。相反，snprintf要求其第二个参数指定目的缓冲区的大小，因此可保证该缓冲区不溢出。必须小心使用的函数还有gets、strcat和strcpy，通常应分别改为fgets、strncat和strncpy。更好的替代函数是后来才引入的strlcat和strlcpy，它们确保结果是正确终止的字符串。

9. 迭代服务器：对于每个客户它都迭代执行一次。同时能处理多个客户端的并发服务器有多种编写技术。最简单的技术是调用Unix的fork函数，为每个客户创建一个子进程。其他技术包括使用线程代替fork，或在服务器启动时预先fork一定数量的子进程。

10. 原始套接字（raw socket）：网络应用绕过传输层直接使用IPv4或IPv6。

11. netstat参数：-i提供网络接口信息，-n输出数值地址，-r展示路由表。环回（loopback）接口称为lo，以太网接口称为eth0。

12. 32位Unix系统上共用的编程模型称为ILP32模型，表示整数（I），长整数（L）和指针（P）都占用32位。64位Unix系统上变得最为流行的模型称为LP64模型，表示只有长整数（L）和指针（P）占用64位。size_t在64位系统中也变为64位值。
