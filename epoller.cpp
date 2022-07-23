#include "epoller.h"
Epoller::Epoller(int maxEvent):
epollerFd(epoll_create(512)),events(maxEvent){
	assert(epollerFd >= 0 && events.size()>0);
}

Epoller::~Epoller(){
	close(epollerFd);
}

bool Epoller::addFd(int fd, uint32_t events){
	if(fd < 0)	return false;
	epoll_event ep = { 0 };
	ep.data.fd = fd;
	ep.events = events;
	return !epoll_ctl(epollerFd, EPOLL_CTL_ADD, fd, &ep);
}

bool Epoller::modFd(int fd, uint32_t events){
	if (fd < 0)	return false;
	epoll_event ep = { 0 };
	ep.data.fd = fd;
	ep.events = events;
	return !epoll_ctl(epollerFd, EPOLL_CTL_MOD, fd, &ep);
}

bool Epoller::delFd(int fd)
{
	if (fd < 0)	return false;
	epoll_event ep = { 0 };
	return !epoll_ctl(epollerFd, EPOLL_CTL_DEL, fd, &ep);
}

int Epoller::wait(int timeoutMs){
	return epoll_wait(epollerFd, &events[0],
		static_cast<int>(events.size()), timeoutMs);
}

int Epoller::getEventFd(size_t i) const
{
	assert(i < events.size() && i >= 0);
	return events[i].data.fd;
}

uint32_t Epoller::getEvents(size_t i) const
{
	assert(i < events.size() && i >= 0);
	return events[i].events;
}

