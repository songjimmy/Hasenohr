#pragma once
#include <muduo/base/Timestamp.h>

#include <functional>
//timer类记录计时器的有效期，工作方式，循环周期，回调函数
class timer
{
public:
	friend struct less_compare;
	typedef std::function<void(void)> callback;
	timer(const muduo::Timestamp& delay,double interval_time,const callback& cb);
	void run();
	bool restart();
	bool check(muduo::Timestamp time_stamp) const;
	muduo::Timestamp waiting_time() const;
private:
	muduo::Timestamp waiting_time_;
	double interval;
	bool repeat;
	const callback cb_;
};
struct less_compare
{
	bool operator()(const timer& timer_1, const timer& timer_2);
};