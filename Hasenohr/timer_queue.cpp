#include "timer_queue.h"

#include <stdio.h>
#include <string.h>
#include <utility>
#include <sys/timerfd.h>
#include <muduo/base/Logging.h>
#include <poll.h>
#include <iostream>
////
int createTimerfd()
{
	int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
		TFD_NONBLOCK | TFD_CLOEXEC);
	if (timerfd < 0)
	{
		LOG_SYSFATAL << "Failed in timerfd_create";
	}
	return timerfd;
}

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
////
struct timespec howMuchTimeFromNow(muduo::Timestamp when)
{
	int64_t microseconds = when.microSecondsSinceEpoch()
		- muduo::Timestamp::now().microSecondsSinceEpoch();
	if (microseconds < 100)
	{
		microseconds = 100;
	}
	struct timespec ts;
	ts.tv_sec = static_cast<time_t>(
		microseconds / muduo::Timestamp::kMicroSecondsPerSecond);
	ts.tv_nsec = static_cast<long>(
		(microseconds % muduo::Timestamp::kMicroSecondsPerSecond) * 1000);
	return ts;
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
////
void resetTimerfd(int timerfd, muduo::Timestamp expiration)
{
	// wake up loop by timerfd_settime()
	struct itimerspec newValue;
	struct itimerspec oldValue;
	bzero(&newValue, sizeof newValue);
	bzero(&oldValue, sizeof oldValue);
	newValue.it_value = howMuchTimeFromNow(expiration);
	int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
	if (ret)
	{
		LOG_SYSERR << "timerfd_settime()";
	}
}
void reset_timer_fd(int timerfd, muduo::Timestamp expiration)
{
	itimerspec howlong = how_much_time_from_now(expiration);
	::timerfd_settime(timerfd, 0, &howlong, NULL);
}


//
void readTimerfd(int timerfd, muduo::Timestamp now)
{
	uint64_t howmany;
	ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
	LOG_INFO << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
	if (n != sizeof howmany)
	{
		LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of "<< sizeof howmany;
	}
}
timer_queue::timer_queue(event_loop* loop)
:
owner_loop_(loop),
time_fd_(create_timer_fd()),
timer_channel(loop, time_fd_),
timer_list_(less_compare())
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
	timer_channel.unenable_reading();
	std::vector<timer> active_timers = pop_front_queue();
	for(auto& item:active_timers)
	{
		item.run();
		if (item.restart()) timer_list_.insert(item);
	}
	set_timer_channel();
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
		auto time_ = timer_list_.begin()->waiting_time();
		LOG_INFO << muduo::timeDifference(time_, muduo::Timestamp::now());
		reset_timer_fd(time_fd_,time_);
		timer_channel.enable_reading();
	}
}

