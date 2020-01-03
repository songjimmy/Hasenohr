#include "event_loop.h"
#include <muduo/base/Thread.h>
#include <stdio.h>
#include <poll.h>
void loop_fun(int time_)
{
	printf("%d\n", pthread_self());
	poll(NULL,0,time_*1000);
}
void thread_fun()
{
	event_loop loop_;
	loop_.set_loop(std::bind(&loop_fun,5));
	loop_.loop();
}
int main_()
{
	event_loop loop_;
	loop_.set_loop(std::bind(&loop_fun,1));
	muduo::Thread thread_(thread_fun);
	thread_.start();
	loop_.loop();
	thread_.join();
	return 0;
}