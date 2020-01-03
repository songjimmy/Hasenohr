#include "event_loop.h"
#include "channel.h"
#include "poller.h"

#include <sys/timerfd.h>
#include <stdio.h>

#include <functional>
void timeout(event_loop* loop)
{
	printf("timeout\n");
	loop->quit();
}

int main()
{
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


	channel_.set_read_callback(std::bind(&timeout,&loop_));
	loop_.loop();
	
}