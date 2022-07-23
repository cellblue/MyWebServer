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

	//��������fd����epoll���
	bool addFd(int fd, uint32_t events);
	//�޸�������fd��Ӧ�ļ��
	bool modFd(int fd, uint32_t events);
	//ɾ��
	bool delFd(int fd);

	int wait(int timewait = -1);

	//��ȡfd
	int getEventFd(size_t i) const;
	//��ȡevents
	uint32_t getEvents(size_t i) const;
private:
	int epollerFd;
	std::vector<struct epoll_event>events;
};
#endif
