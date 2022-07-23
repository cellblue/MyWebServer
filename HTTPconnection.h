#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H

#include <arpa/inet.h>
#include <sys/uio.h>
#include <iostream>
#include <sys/types.h>
#include <assert.h>


#include "buffer.h"
#include "HTTPrequest.h"
#include "HTTPresponse.h"

class HTTPconnection{
private:
	int fd;//��Ӧ������
	struct sockaddr_in addr;
	bool isClose;
	int iovCnt;
	struct iovec iov[2];

	//������
	Buffer _readBuffer;
	Buffer _writeBuffer;

	HTTPrequest request;
	HTTPresponse response;


public:
	static bool isET;
	static const char* srcDir;
	static std::atomic<int> userCount;


	HTTPconnection();
	~HTTPconnection();
	void initHTTPconnection(int fd,const sockaddr_in& addr);
	
	//��д�ӿ�
	ssize_t readBuffer(int* saveErrno);
	ssize_t writeBuffer(int* saveErrno);

	//�ر�����
	void closeHTTPConnection();
	//��������
	bool handleHTTPConnection();

	const char* getIP() const;
	int getPort() const;
	int getFd() const;
	sockaddr_in getAddr() const;

	int writeBytes(){return iov[1].iov_len + iov[0].iov_len;}

	bool isKeepAlive() const{return request.isKeepAlive();}
};
#endif
