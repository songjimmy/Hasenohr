#include "timer_queue.h"

#include <stdio.h>
#include <string.h>
#include <utility>
#include <sys/timerfd.h>

timer_queue::timer_queue(event_loop* loop)
:time_fd_(::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),timer_channel(loop,time_fd_),
owner_loop_(loop), timer_list_(less_compare())
{
	
}

void timer_queue::add_timer_queue(const timer& timer_)
{
	timer_list_.insert(timer_);
}

std::vector<timer> timer_queue::pop_front_queue()
{
	std::vector<timer> ret_timers;
	muduo::Timestamp time_now = muduo::Timestamp::now();
	while (!timer_list_.empty()&&(timer_list_.begin()->check(time_now)))
	{
		ret_timers.push_back(*(timer_list_.begin()));
		timer_list_.erase(*(timer_list_.begin()));
	}
	return ret_timers;
}

