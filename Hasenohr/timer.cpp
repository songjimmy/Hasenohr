#include "timer.h"

timer::timer(const muduo::Timestamp& delay, double interval_time, const callback& cb)
	:waiting_time(delay),interval(interval_time),repeat(interval_time>0),cb_(cb)
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
		waiting_time = muduo::addTime(waiting_time, interval);
		return true;
	}
	else
	{
		waiting_time = muduo::Timestamp::invalid();
		return false;
	}
}

bool timer::check(muduo::Timestamp time_stamp) const
{
	return time_stamp > waiting_time;
}

bool less_compare::operator()(const timer& timer_1, const timer& timer_2)
{
	if (timer_1.waiting_time < timer_2.waiting_time) return true;
	else if (timer_2.waiting_time < timer_1.waiting_time) return false;
	else if (&timer_1 < &timer_2) return true;
	else return false;
}
