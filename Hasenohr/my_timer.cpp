#include "my_timer.h"
#include <stdio.h>
#include <string.h>
#include "channel.h"
#include "event_loop.h"
#include <utility>
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

my_timer::my_timer(int time_ms, event_loop* owner_loop)
	:owner_loop_(owner_loop), time_fd_(::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC))
, channel_(owner_loop,time_fd_)
{
	printf("%d\n", time_fd_);
	bzero(&howlong, sizeof howlong);
	howlong.it_value.tv_sec = time_ms / muduo::Timestamp::kMicroSecondsPerSecond;
	howlong.it_value.tv_nsec = time_ms % muduo::Timestamp::kMicroSecondsPerSecond * 1000;
	::timerfd_settime(time_fd_, 0, &howlong, NULL);
}

my_timer::~my_timer()
{
	close(time_fd_);
	LOG_TRACE << "timer "<< time_fd_ << " is closed";
}

void my_timer::set_time_callback(const callback& cb)
{
	//边缘触发，传入的方法在计时器触发后执行一次
	auto time_function = [cb,this](channel& channel__)->void 
	{
		uint64_t howmany;
		ssize_t n = ::read(time_fd_, (void*)(&howmany), sizeof howmany);
		LOG_INFO << "TimerQueue::handleRead() " << howmany;
		if (n != sizeof howmany)
		{
			LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of "<< sizeof howmany;
		}
		cb();
	};
	channel_.set_read_callback(std::bind(time_function,std::ref(channel_)));
	channel_.enable_reading();
}
