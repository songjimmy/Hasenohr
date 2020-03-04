#pragma once
#include <muduo/base/Timestamp.h>

#include <functional>
#include <memory>
//timer类记录计时器的有效期，工作方式，循环周期，回调函数
class timer:public std::enable_shared_from_this<timer>
{
public:
	friend struct less_compare;
	typedef std::function<void(void)> callback;
	timer(const muduo::Timestamp& delay,double interval_time,const callback& cb);
	void run();
	//将其循环标志位置为false 动作置为空 使之走完当前的循环就析构
	void set_invalid();
	bool restart();
	bool check(muduo::Timestamp time_stamp) const;
	muduo::Timestamp waiting_time() const;
private:
	static const callback invaild_cb_ ;
	muduo::Timestamp waiting_time_;
	double interval;
	bool repeat;
	callback cb_;
};
struct less_compare
{
	bool operator()(const std::shared_ptr<timer>& timer_1, const std::shared_ptr<timer>& timer_2);
};
