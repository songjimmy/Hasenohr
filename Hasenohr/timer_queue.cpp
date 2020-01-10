#include "timer_queue.h"

#include <stdio.h>
#include <string.h>
#include <utility>
#include <sys/timerfd.h>
#include <muduo/base/Logging.h>
#include <iostream>
//

int create_timer_fd()
{
	int timer_fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	printf("%d\n", timer_fd);
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
	int64_t howlong_microseconds = when.microSecondsSinceEpoch() - muduo::Timestamp::now().microSecondsSinceEpoch();
	howlong.it_value.tv_sec = howlong_microseconds/muduo::Timestamp::kMicroSecondsPerSecond;
	howlong.it_value.tv_nsec = howlong_microseconds% muduo::Timestamp::kMicroSecondsPerSecond*1000;
	return howlong;
}

void reset_timer_fd(int timerfd, muduo::Timestamp expiration)
{
	itimerspec howlong;
	bzero(&howlong, sizeof howlong);
	howlong = how_much_time_from_now(expiration);
	::timerfd_settime(timerfd, 0, &howlong, NULL);
}

void readTimerfd(int timerfd, muduo::Timestamp now)
{
	uint64_t howmany;
	ssize_t n = ::read(timerfd, (void*)(&howmany), sizeof howmany);
	//LOG_ERROR << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
	if (n != sizeof howmany)
	{
		//LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of "<< sizeof howmany;
	}
}
timer_queue::timer_queue(event_loop* loop)
:
timer_list_(less_compare()),	
timer_channel(loop, time_fd_),
time_fd_(create_timer_fd()),
owner_loop_(loop)
{
	timer_channel.print();
	timer_channel.set_read_callback(std::bind(&timer_queue::handle_alarm, this));
	timer_channel.enable_reading();
	timer_channel.print();
}

void timer_queue::add_timer(const timer& timer_)
{
	timer_list_.insert(timer_);
}

std::vector<timer> timer_queue::pop_front_queue()
{
	muduo::Timestamp time_now = muduo::Timestamp::now();
	timer_set::iterator it=timer_list_.upper_bound(timer(time_now,0,NULL));
	std::vector<timer> ret_timers(timer_list_.begin(),it);
	timer_list_.erase(timer_list_.begin(), it);
	return ret_timers;
}

void timer_queue::handle_alarm()
{
	readTimerfd(time_fd_, muduo::Timestamp::now());
	std::vector<timer> active_timers = pop_front_queue();
	for(auto& item:active_timers)
	{
		item.run();
		if (item.restart()) timer_list_.insert(item);
	}
	set_timer_channel();
	timer_channel.enable_reading();
}

timer_queue::~timer_queue()
{
	close(time_fd_);
}

void timer_queue::set_timer_channel()
{
	if (timer_list_.empty())
	{
	}
	else
	{
		reset_timer_fd(time_fd_,timer_list_.begin()->waiting_time());
	}
}

