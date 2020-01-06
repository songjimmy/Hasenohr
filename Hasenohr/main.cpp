#include "event_loop.h"
#include "channel.h"
#include "poller.h"
#include "my_timer.h"
#include "timer_queue.h"

#include <muduo/base/Thread.h>

#include <sys/timerfd.h>
#include <stdio.h>

#include <functional>
void timeout_1()
{
	printf("time out 1\n");
}

void timeout_2()
{
	printf("time out 2\n");
}

void timeout_3()
{
	printf("time out 3\n");
}
int main_1()
{
	//
	int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);//flag
	event_loop loop_;
	channel channel_(&loop_,timerfd);
	channel_.enable_reading();

	//
	struct itimerspec howlong;
	//memset
	//memset(&howlong, 0, sizeof howlong);
	bzero(&howlong, sizeof howlong);
	howlong.it_value.tv_sec = 5;
	::timerfd_settime(timerfd, 0, &howlong, NULL);


	channel_.set_read_callback(std::bind(&timeout_1));
	loop_.loop();
	return 1;
}
void thread_function()
{
	int i = 0;
	while (true)
	{
		printf("%dsecond\n",i);
		sleep(1);
		++i;
	}
}

int main_2()
{
	muduo::Thread thread_(thread_function,"timer");
	event_loop loop_;
	my_timer timer_(5, &loop_);
	timer_.set_time_callback( std::bind(&timeout_1));
	my_timer timer_2(10, &loop_);
	timer_2.set_time_callback(std::bind(&timeout_1));
	thread_.start();
	loop_.loop();
	return 0;
}

int main()
{
	muduo::Thread thread_(thread_function, "timer");
	event_loop loop_;
	timer_queue timer_queue_(&loop_);
	timer timer_1(muduo::addTime(muduo::Timestamp::now(), 1), 1.5 , &timeout_1);
	timer timer_2(muduo::addTime(muduo::Timestamp::now(), 2), 0   , &timeout_2);
	timer timer_3(muduo::addTime(muduo::Timestamp::now(), 3), 0   , &timeout_3);
	timer_queue_.add_timer(timer_1);
	timer_queue_.add_timer(timer_2);
	timer_queue_.add_timer(timer_3);
	my_timer timer_(5, &loop_);
	timer_.set_time_callback(std::bind(&timeout_1));
	my_timer timer__(10, &loop_);
	timer__.set_time_callback(std::bind(&timeout_2));
	timer_queue_.set_timer_channel();
	//thread_.start();
	loop_.loop();
}