#include "timer.h"

timer::timer(const muduo::Timestamp& delay, double interval_time, const callback& cb)
	:waiting_time_(delay),interval(interval_time),repeat(interval_time>0),cb_(cb)
{
}

void timer::run()
{
	cb_();
}

bool timer::restart()
{
	if (repeat)
	{
		waiting_time_ = muduo::addTime(waiting_time_, interval);
		return true;
	}
	else
	{
		waiting_time_ = muduo::Timestamp::invalid();
		return false;
	}
}

bool timer::check(muduo::Timestamp time_stamp) const
{
	return time_stamp > waiting_time_;
}

muduo::Timestamp timer::waiting_time() const
{
	return waiting_time_;
}

bool less_compare::operator()(const timer& timer_1, const timer& timer_2)
{
	if (timer_1.waiting_time_ < timer_2.waiting_time_) return true;
	else if (timer_2.waiting_time_ < timer_1.waiting_time_) return false;
	else if (&timer_1 < &timer_2) return true;
	else return false;
}
