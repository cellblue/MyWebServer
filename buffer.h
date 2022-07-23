#ifndef BUFFER_H
#define BUFFER_H

#include<vector>
#include<iostream>
#include<cstring>
#include<atomic>
#include<unistd.h>
#include<sys/uio.h>
#include<assert.h>



class Buffer{
private:
	char* BeginPtr();
	const char* BeginPtr() const;
	
	//扩容
	void allocateSpace(size_t len);

	std::vector<char> buffer; //buffer实体
	std::atomic<size_t> readPos;//读指针
	std::atomic<size_t> writePos;//写指针
public:
	Buffer(int BufferSize = 1024);
	~Buffer() = default;

	//可以读取的字节数
	size_t writeAbleBytes() const;
	//可以写入的字节数
	size_t readAbleBytes() const;
	//已经读取的字节数
	size_t readBytes() const;

	//获取读指针
	const char* curReadPtr() const;
	//获取写指针
	char* curWritePtr();
	const char* curWritePtrConst()const;
	//更新读指针
	void updateReadPtr(size_t len);
	//更新到now位置
	void updateReadPtrToNow(const char* now);

	//更新写指针
	void updateWritePtr(size_t len);
	//指针初始化
	void initPtr();

	//保证将数据写入缓冲区
	void ensureWrite(size_t len);
	//写入
	void append(const char* str,size_t len);
	void append(const std::string& str);
	void append(const void* data,size_t len);
	void append(const Buffer& buffer);

	//IO接口
	ssize_t readFd(int fd, int* Errno);
	ssize_t writeFd(int fd, int* Errno);

	std::string BuffertoStr();

	//测试
	void print() {
		
	}
};

#endif
