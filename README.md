# Library-Management-System

该程序主要包含一个客户端和一个服务器端。实现了基本的数据库增删改查，MVC的使用以及基本的TCP网络编程等。

图书作为一个临界资源，该程序未添加锁操作；服务器端程序也并没有添加多线程来应对多用户访问。程序代码冗余较多，比如在Admin和User之上，应该有一个用户基类。在网络通信这一部分，我使每一个窗口类（Dialog或者MainWindow）负责其自身的通信内容，这造成了许多代码的重复。可以单独派生一个TcpSocket，该类负责所有通信内容。

