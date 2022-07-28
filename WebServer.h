#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <unordered_map>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory>
#include <iostream>

#include "epoller.h"
#include "timer.h"
#include "threadpool.h"
#include "HTTPconnection.h"
#include "SqlConnectPool.h"

class WebServer{
public:
	WebServer(
	int port, int trigMode, int timeoutMs, bool optLinger, 
	int sqlPort, const char* sqlUser, const  char* sqlPwd,const char* dbName,int sqlSize,
	int threadNum); 
	~WebServer();
	void Start();
private:

	int port;
	int timeoutMS;//默认超时时间
	bool isClose;
	int listenFd;
	bool openLinger;//优雅关闭
	char* srcDir;

	uint32_t listenEvent;
	uint32_t connectionEvent;

	std::unique_ptr<TimerManager>timer;
	std::unique_ptr<ThreadPool> threadpool;
	std::unique_ptr<Epoller> epoller;
	std::unordered_map<int, HTTPconnection> users;
	
	static const int MAX_FD = 65536;
	static int setFdNonblock(int fd);

	bool initSocket();
	void initEventMode(int trigMode);

	void addHTTP(int fd, sockaddr_in addr);
	void closeHTTP(HTTPconnection* client);

	//监听
	void handleListen();
	//http的IO读写
	void handleRead(HTTPconnection* client);
	void handleWrite(HTTPconnection* client);
	//读写进线程池
	void onRead(HTTPconnection* client);
	void onWrite(HTTPconnection* client);
	void onProcess(HTTPconnection* client);

	void sendError(int fd, const char* info);
	void extentTime(HTTPconnection* client);

};
#endif
