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
	int id;//��Ƕ�ʱ��
	TimeStamp expire;//����ʱ��
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
	//��������¼�
	void handleExpired();
	//��һ�δ���
	int nextHandle();

	void update(int id,int timeout);
	void work(int fd);

	void clear() { Set.clear(); };

private:
	std::set<TimerNode>Set;
	std::unordered_map<int, TimerNode> Map;
};
#endif //TIMER_H
