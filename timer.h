#ifndef TIMER_H
#define TIMER_H

#include <queue>
#include <deque>
#include <set>
#include <ctime>
#include <chrono>
#include <functional>
#include <memory>
#include <unordered_map>
#include "HTTPconnection.h"

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;


class TimerNode
{
public:
	int id;//标记定时器
	TimeStamp expire;//过期时间
	TimeoutCallBack tcb;
	bool operator < (const TimerNode& t)const {
		return this->expire < t.expire;
	}
};
//typedef std::shared_ptr<TimerNode> SP_TimerNode;
class TimerManager {
public:
	~TimerManager();

	void addTimer(int id, int timeout, const TimeoutCallBack& cb);
	//处理过期事件
	void handleExpired();
	//下一次处理
	int nextHandle();

	void update(int id,int timeout);
	void work(int fd);

	void clear() { Set.clear(); };

private:
	std::set<TimerNode>Set;
	std::unordered_map<int, TimerNode> Map;
};
#endif //TIMER_H
