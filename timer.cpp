#include "timer.h"

TimerManager::~TimerManager()
{
	clear();
}
void TimerManager::addTimer(int id, int timeout, const TimeoutCallBack& cb)
{
	assert(id >= 0);
	if (Set.count(Map[id])) {
		update(id,timeout);
	}
	else {
		Map[id].expire = Clock::now() + MS(timeout);
		Set.insert(Map[id]);
	}
}
void TimerManager::handleExpired()
{
	if (Set.empty()) return;
	while (!Set.empty()) {
		TimerNode node = *Set.begin();
		if (std::chrono::duration_cast<MS>(node.expire - Clock::now()).count() > 0) {
			break;
		}
		node.tcb();
		Set.erase(node);
	}
}
int TimerManager::nextHandle()
{
	handleExpired();
	size_t res = -1;
	if (!Set.empty()) {
		res = std::chrono::duration_cast<MS>(Set.begin()->expire - Clock::now()).count();
		res = res < 0 ? 0 : res;
	}
	return res;
}
void TimerManager::update(int id, int timeout)
{
	assert(Set.size() != 0 && Set.count(Map[id]) > 0);
	TimerNode tmp(Map[id]);
	tmp.expire = Clock::now() + MS(timeout);
	Set.erase(Map[id]);
	Set.insert(tmp);
}

void TimerManager::work(int fd)
{
	if (!Set.empty() || Map.count(fd) == 0) return;
	Map[fd].tcb();
	Set.erase(Map[fd]);
}
