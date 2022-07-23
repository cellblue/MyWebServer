#ifndef EPOLLER_H
#define EPOLLER_H

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <vector>
#include <errno.h>
#include <cstdint>
#include <cstddef>

class Epoller{
public:
	explicit Epoller(int maxEvent = 1024);
	~Epoller();

	//将描述符fd加入epoll监控
	bool addFd(int fd, uint32_t events);
	//修改描述符fd对应的监控
	bool modFd(int fd, uint32_t events);
	//删除
	bool delFd(int fd);

	int wait(int timewait = -1);

	//获取fd
	int getEventFd(size_t i) const;
	//获取events
	uint32_t getEvents(size_t i) const;
private:
	int epollerFd;
	std::vector<struct epoll_event>events;
};
#endif
