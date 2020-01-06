#include "my_timer.h"
#include <stdio.h>
#include <string.h>
#include "channel.h"
#include "event_loop.h"
#include <utility>
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
my_timer::my_timer(int time_ms, event_loop* owner_loop):owner_loop_(owner_loop), time_fd_(::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC))
, channel_(owner_loop,time_fd_)
{
	bzero(&howlong, sizeof howlong);
	howlong.it_value.tv_sec = time_ms ;
	::timerfd_settime(time_fd_, 0, &howlong, NULL);
}

my_timer::~my_timer()
{
	close(time_fd_);
	LOG_TRACE << "timer "<< time_fd_ << " is closed";
}

void my_timer::set_time_callback(const callback& cb)
{
	//��Ե����������ķ����ڼ�ʱ��������ִ��һ��
	auto time_function = [cb](channel& channel__)->void { channel__.unenable_reading(); cb();};
	channel_.set_read_callback(std::bind(time_function,std::ref(channel_)));
	channel_.enable_reading();
}
