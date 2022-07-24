# MyWebServer

- 利用IO复用技术Epoll与线程池实现多线程的Reactor高并发模型；
- 利用正则与状态机解析HTTP请求报文，实现处理静态资源的请求；
- 利用标准库容器封装char，实现自动增长的缓冲区；
- 基于std::set实现的定时器，关闭超时的非活动连接；

项目启动
```
make
./bin/myserver
```
压力测试：
```
./webbench-1.5/webbench -c 100 -t 10 http://127.0.0.1:1416/
./webbench-1.5/webbench -c 1000 -t 10 http://127.0.0.1:1416/
./webbench-1.5/webbench -c 5000 -t 10 http://127.0.0.1:1416/
./webbench-1.5/webbench -c 10000 -t 10 http://127.0.0.1:1416/
```
