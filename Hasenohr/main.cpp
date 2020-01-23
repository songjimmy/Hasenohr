#include "event_loop.h"
#include "channel.h"
#include "poller.h"
#include "my_timer.h"
#include "timer_queue.h"
#include "event_loop_thread.h"
#include "acceptor.h"
#include "tcp_server.h"

#include <muduo/base/Thread.h>

#include <sys/timerfd.h>
#include <stdio.h>


#include <functional>
using std::placeholders::_1;
using std::placeholders::_2;
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
void timeout_4()
{
	printf("time out 4\n");
}
void timeout_5()
{
	printf("time out 5\n");
}
void timeout_6()
{
	printf("time out 6\n");
}
void timeout_7(tcp_connection_ptr)
{
	printf("time out 7\n");
}
void timeout_8(const socket_obj&,const event_loop&)
{
	printf("time out 7\n");
}
void timeout_9(tcp_connection_ptr,std::string buf)
{
	printf(buf.c_str());
	printf("recive\n");
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
		if (i == 5)
		{
		}
	}
}

void thread_function_1(event_loop* loop)
{
	int i = 0;
	while (true)
	{
		printf("%dsecond\n", i);
		sleep(1);
		++i;
		if (i == 5)
		{
			loop->run_after(&timeout_6,1);
		}
		if (i == 10)
		{
			loop->quit();
		}
	}
}

int main_2()
{
	muduo::Thread thread_(thread_function,"timer");
	event_loop_thread loop_thread;
	event_loop* loop_ = loop_thread.loop();
	my_timer timer_(5, loop_);
	timer_.set_time_callback( std::bind(&timeout_1));
	my_timer timer_2(10, loop_);
	timer_2.set_time_callback(std::bind(&timeout_1));
	thread_.start();
	loop_->loop();
	return 0;
}

int main_3()
{
	muduo::Thread thread_(thread_function, "timer");
	event_loop loop_;
	
	timer_queue timer_queue_(&loop_);
	timer timer_1(muduo::addTime(muduo::Timestamp::now(), 0.5), 1   , &timeout_1);
	timer timer_2(muduo::addTime(muduo::Timestamp::now(), 2), 0   , &timeout_2);
	timer timer_3(muduo::addTime(muduo::Timestamp::now(), 3), 0   , &timeout_3);
	timer_queue_.add_timer(timer_1);
	timer_queue_.add_timer(timer_2);
	timer_queue_.add_timer(timer_3);
	timer_queue_.set_timer_channel();
	thread_.start();
	loop_.loop();
	return 1;
}

int main_4()
{
	event_loop_thread loop_thread;
	event_loop* loop_ = loop_thread.loop();
	loop_->run_after(&timeout_1, 9);
	loop_->run_every(&timeout_2, 6);
	loop_->run_every(&timeout_3, 3);
	muduo::Thread thread_(std::bind(&thread_function_1,loop_), "timer");
	thread_.start();
	thread_.join();
	return 1;
}
void daytime(int fd, event_loop& loop)
{
	std::string now_str = muduo::Timestamp::now().toFormattedString();
	::write(fd, now_str.c_str(), now_str.size());
	close(fd);
}
void welcome(int fd, event_loop& loop)
{
	std::string str("hello tcp_client");
	::write(fd, str.c_str(), str.size());
	::close(fd);
}
int main()
{
	event_loop loop_;
	socket_obj socket_1(8888);
	tcp_server server(&loop_,socket_1,"server");
	server.set_connection_callback(timeout_7);
	server.set_message_callback(timeout_9);
	server.start();
	loop_.loop();
	return 1;
}