#include "timer_queue.h"

#include <stdio.h>
#include <string.h>
#include <utility>
#include <sys/timerfd.h>
//

int create_timer_fd()
{
	int timer_fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	if (!timer_fd)
	{
		printf("Create timer_fd unsuccessfully");
	}
	return timer_fd;
}

itimerspec how_much_time_from_now(muduo::Timestamp when)
{
	itimerspec howlong;
	bzero(&howlong, sizeof howlong);
	howlong;
	int64_t howlong_microseconds = when.microSecondsSinceEpoch() - muduo::Timestamp::now().microSecondsSinceEpoch();
	howlong.it_value.tv_sec = howlong_microseconds/muduo::Timestamp::kMicroSecondsPerSecond;
	howlong.it_value.tv_nsec = howlong_microseconds% muduo::Timestamp::kMicroSecondsPerSecond*1000;
	return howlong;
}

void reset_timer_fd(int timerfd, muduo::Timestamp expiration)
{
	itimerspec howlong = how_much_time_from_now(expiration);
	::timerfd_settime(timerfd, 0, &howlong, NULL);
}

struct time_function
{
	void operator()(timer_queue& timer_queue_, channel& channel__)
	{
		channel__.unenable_reading();
		timer_queue_.handle_alarm();
		timer_queue_.set_timer_channel();
	}
};
timer_queue::timer_queue(event_loop* loop)
:time_fd_(create_timer_fd()),timer_channel(loop,time_fd_),
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
	if (timer_list_.empty())
	{
		timer_channel.unenable_reading();
	}
	else
	{
		reset_timer_fd(time_fd_,timer_list_.begin()->waiting_time());
		timer_channel.set_read_callback(std::bind(time_function(), std::ref(*this), std::ref(this->timer_channel)));
		timer_channel.enable_reading();
	}
}

