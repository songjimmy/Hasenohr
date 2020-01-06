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

void timer_queue::add_timer(const timer& timer_)
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

void timer_queue::handle_alarm()
{
	std::vector<timer> active_timers = pop_front_queue();
	for(auto& item:active_timers)
	{
		item.run();
		if (item.restart()) timer_list_.insert(item);
	}
}



timer_queue::~timer_queue()
{
	close(time_fd_);
}

void timer_queue::set_timer_channel()
{
	bzero(&howlong, sizeof howlong);
	if (timer_list_.empty())
	{
		timer_channel.unenable_reading();
	}
	else
	{
		double temp= muduo::timeDifference(timer_list_.begin()->waiting_time(), muduo::Timestamp::now());
		howlong.it_value.tv_sec = temp;
		howlong.it_value.tv_nsec = (muduo::timeDifference(timer_list_.begin()->waiting_time(), muduo::Timestamp::now()) - howlong.it_value.tv_sec) * double(1000000000);
		::timerfd_settime(time_fd_, 0, &howlong, NULL);
		auto time_function = [](timer_queue& timer_queue_,channel& channel__)
		{	
			channel__.unenable_reading();
			timer_queue_.handle_alarm(); 
			timer_queue_.set_timer_channel(); 
		};
		timer_channel.set_read_callback(std::bind(time_function,std::ref(*this), std::ref(timer_channel)));
		timer_channel.enable_reading();
		
	}
}

