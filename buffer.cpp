#include "buffer.h"

char* Buffer::BeginPtr()
{
	return &*buffer.begin();
}

const char* Buffer::BeginPtr() const
{
	return &*buffer.begin();
}

void Buffer::allocateSpace(size_t len)
{
	if (writeAbleBytes()+readBytes() < len) {
		buffer.resize(writePos + len + 1);
	}
	else {
		size_t readAble = readAbleBytes();
		std::copy(BeginPtr()+readPos,BeginPtr()+writePos,BeginPtr());
		readPos = 0;
		writePos = readAble;
		assert(readAble == readAbleBytes());
	
	}
}

Buffer::Buffer(int BufferSize):buffer(BufferSize),readPos(0), writePos(0)
{}

size_t Buffer::readAbleBytes() const
{
	return writePos - readPos;
}

size_t Buffer::writeAbleBytes() const
{
	return buffer.size() - writePos;
}

size_t Buffer::readBytes() const
{
	return readPos;
}

const char* Buffer::curReadPtr() const
{
	return BeginPtr()+readPos;
}

char* Buffer::curWritePtr()
{
	return BeginPtr() + writePos;
}
const char* Buffer::curWritePtrConst() const
{
	return BeginPtr() + writePos;
}

void Buffer::updateReadPtr(size_t len)
{
	assert(len <= readAbleBytes());
	readPos += len;
}

void Buffer::updateReadPtrToNow(const char* now)
{
	assert(now >= curReadPtr());
	updateReadPtr(now-curReadPtr());
}

void Buffer::updateWritePtr(size_t len)
{
	assert(len <= writeAbleBytes());
	writePos += len;
}

void Buffer::initPtr()
{
	memset(&buffer, 0, sizeof buffer);
	readPos = 0;
	writePos = 0;
}

void Buffer::ensureWrite(size_t len)
{
	if (writeAbleBytes() < len) {
		allocateSpace(len);
	}
	assert(writeAbleBytes() >= len);
}

void Buffer::append(const char* str, size_t len)
{
	assert(str);
	ensureWrite(len);
	std::copy(str,str+len,curWritePtr());
	updateWritePtr(len);
}

void Buffer::append(const std::string& str)
{
	append(str.data(),str.length());
}

void Buffer::append(const void* data, size_t len)
{
	assert(data);
	append(static_cast<const char*> (data), len);
}

void Buffer::append(const Buffer& buffer)
{
	append(buffer.curReadPtr(),buffer.readAbleBytes());
}

ssize_t Buffer::readFd(int fd, int* Errno)
{
	char buff[65535];//缓冲区
	struct iovec iov[2];
	//还可以读的数据
	const size_t writeAble = writeAbleBytes();
	
	iov[0].iov_base = BeginPtr() + writePos;
	iov[0].iov_len = writeAble;

	iov[1].iov_base = buff;
	iov[1].iov_len = sizeof(buff);

	const ssize_t len = readv(fd,iov,2);
	if (len < 0) {
		//std::cout<<"fd read fail!"<<std::endl;
		*Errno = errno;
		//std::cout<<"The errno is:"<<errno<<std::endl;
	}
	else if (static_cast<size_t>(len) <= writeAble) {
		writePos += len;
	}
	else {
		writePos = buffer.size();
		append(buff,len-writeAble);
	}
	return len;
}

ssize_t Buffer::writeFd(int fd, int* Errno)
{
	size_t readAble = readAbleBytes();
	ssize_t len = write(fd,curReadPtr(),readAble);
	if (len < 0) {
		//std::cout<<"fd write fail!"<<std::endl;
		*Errno = errno;
		//std::cout<<"The errno is:"<<errno<<std::endl;
		return len;
	}
	readAble += len;
	return len;
}

std::string Buffer::BuffertoStr()
{
	std::string str(curReadPtr(),readAbleBytes());
	initPtr();
	return str;
}

