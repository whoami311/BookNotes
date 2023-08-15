# 第二章

1. TCP：传输控制协议（Transmission Control Protocol）。TCP是一个面向连接的协议，为用户进程提供可靠的全双工字节流。TCP套接字是一种流套接字（stream socket）。TCP关心确认、超时和重传之类的细节。

2. UDP：用户数据报协议（User Datagram Protocol）。UDP是一个无连接协议。UDP套接字是一种数据报套接字（datagram socket）。UDP数据报不能保证最终到达它们的目的地。

3. ICMP：网际控制消息协议（Internet Control Message Protocol）。ICMP处理在路由器和主机之间流通的错误和控制消息。这些消息通常由TCP/IP网络支持软件本身（而不是用户进程）产生和处理，不过ping和traceroute程序同样使用ICMP。

4. IGMP：网际组管理协议（Internet Group Management Protocol）。IGMP用于多播，它在IPv4中是可选的。

5. ARP：地址解析协议（Address Resolution Protocol）。ARP把一个IPv4地址映射成一个硬件地址。ARP通常用于诸如以太网、令牌环网和FDDI等广播网络，在点到点网络上并不需要。

6. RARP：反向地址解析协议（Reverse Address Resolution Protocol）。RARP把一个硬件地址映射成一个IPv4地址。它有时用于无盘节点的引导。

7. TCP特点：确认、序列号、RTT估算、超时和重传机制、流量控制、拥塞控制、全双工。

8. TCP选项：MSS（maximum segment size，最大分节大小）选项，发送SYN的TCP一端使用本选项通告对端它的最大分节大小，也就是它在本连接的每个TCP分节中愿意接受的最大数据量。发送端TCP使用接收端的MSS值作为所发送分节的最大大小。窗口规模选项，TCP连接任何一端能够通告对端的最大窗口大小是65535，因为在TCP首部中相应的字段占16位。这个新选项指定TCP首部中的通告窗口必须扩大（即左移）的位数（0~14），因此所提供的最大窗口接近1GB（65535*2^14）。在一个TCP连接上使用窗口规模的前提是它的两个端系统都必须支持这个选项。时间戳选项，这个选项对于高速网络连接是必要的，它可以防止由失而复现的分组可能造成的数据破坏。

9. 当一个Unix进程终止时，所有打开的描述符都被关闭，这也导致仍然打开的任何TCP连接上也发出一个FIN。

10. TCP连接中执行主动关闭的那端经历TIME_WAIT状态，该端点停留在这个状态的持续时间是最长分节生命期（maximum segment lifetime，MSL）的两倍，称之为2MSL。任何TCP实现都必须为MSL选择一个值。

11. TIME_WAIT状态两个存在的理由：（1）可靠地实现TCP全双工连接的终止；（2）允许老的重复分节在网络中消逝。

12. RFC3232将端口号划分成以下3段。（1）众所周知的端口为0~1023。（2）已登记的端口为1024~49151。（3）49152~65535是动态的或私用的端口。也叫临时端口（49152是65536的四分之三）。

13. 影响IP数据报大小的限制：
- IPv4数据报的最大大小是65535字节，包括IPv4首部。
- IPv6数据报的最大大小是65575字节，包括40字节的IPv6首部。
- 许多网络有一个可由硬件规定的MTU。
- 在两个主机之间的路径中最小的MTU称为路径MTU（path MTU）。
- 当一个IP数据报将从某个接口送出时，如果它的大小超过相应链路的MTU，IPv4和IPv6都将执行分片（fragmentation）。
- IPv4首部的“不分片(don't fragment)”位（即DF位）若被设置，那么不管是发送这些数据报的主机还是转发它们的路由器，都不允许对它们分片。
- IPv4和IPv6都定义了最小重组缓冲区大小（minimum reassembly buffer size），它是IPv4或IPv6的任何实现都必须保证支持的最小数据报大小。其值对于IPv4为576字节，对于IPv6为1500字节。
- TCP有一个MSS（maximum segment size，最大分节大小），用于向对端TCP通告对端在每个分节中能发送的最大TCP数据量。

14. UDP是一个简单、不可靠、无连接的协议，而TCP是一个复杂、可靠、面向连接的协议。
