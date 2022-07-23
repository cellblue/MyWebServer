#include "HTTPconnection.h"

const char* HTTPconnection::srcDir;
std::atomic<int> HTTPconnection::userCount;
bool HTTPconnection::isET;
HTTPconnection::HTTPconnection()
{
	fd = -1;
    	addr = { 0 };
    	isClose = true;
}

HTTPconnection::~HTTPconnection()
{
	closeHTTPConnection();
}

void HTTPconnection::initHTTPconnection(int fd, const sockaddr_in& addr)
{
	assert(fd > 0);
	userCount++;
	this->addr = addr;
	this->fd = fd;
	_readBuffer.initPtr();
	_writeBuffer.initPtr();
	isClose = false;
}

ssize_t HTTPconnection::readBuffer(int* saveErrno)
{
	ssize_t len = -1;
	do {
		len = _readBuffer.readFd(fd,saveErrno);
		//std::cout<<"Errno:"<<*saveErrno<<std::endl;
	} while (isET && len > 0);
	return len;
}

ssize_t HTTPconnection::writeBuffer(int* saveErrno)
{
	ssize_t len = -1;
	do {
		len = writev(fd,iov,iovCnt);
		if (len <= 0) {
			*saveErrno = errno;
			break;
		}
		if (iov[0].iov_len + iov[1].iov_len == 0) break;
		else if (static_cast<size_t>(len) > iov[0].iov_len) {
			iov[1].iov_base = static_cast<uint8_t*>(iov[1].iov_base) + (len - iov[0].iov_len);
			iov[1].iov_len -= (len - iov[0].iov_len);
			if (iov[0].iov_len) {
				_writeBuffer.initPtr();
				iov[0].iov_len = 0;
			}
		}
		else {
			iov[0].iov_base = static_cast<uint8_t*>(iov[1].iov_base) + len;
			iov[0].iov_len -= len;
			_writeBuffer.updateReadPtr(len);
		}
	} while (isET || iov[0].iov_len + iov[1].iov_len > 10240);
	return len;
}

void HTTPconnection::closeHTTPConnection()
{
	response.unmapFile();
	if (!isClose) {
		isClose = true;
		userCount--;
		close(fd);
	}
}

bool HTTPconnection::handleHTTPConnection()
{
	request.init();
	if (_readBuffer.readAbleBytes() <= 0) {
		std::cout<<"readBuffer is empty!"<<std::endl;		
		return false;
	}
	else if (request.parse(_readBuffer)) {
		response.init(srcDir,request.path(),request.isKeepAlive(),200);
	}
	else {
		std::cout << "400!" << std::endl;
		response.init(srcDir, request.path(), false, 400);
	}

	response.makeResponse(_writeBuffer);
	
	iov[0].iov_base = const_cast<char*>(_writeBuffer.curReadPtr());
	iov[0].iov_len = _writeBuffer.readAbleBytes();
	iovCnt = 1;

	if (response.fileLen() > 0 && response.file()) {
		iov[1].iov_base = response.file();
		iov[1].iov_len = response.fileLen();
		iovCnt = 2;
	}
	return true;
}

const char* HTTPconnection::getIP() const
{
	return inet_ntoa(addr.sin_addr);
}

int HTTPconnection::getPort() const
{
	return addr.sin_port;
}

int HTTPconnection::getFd() const
{
	return fd;
}

sockaddr_in HTTPconnection::getAddr() const
{
	return addr;
}
