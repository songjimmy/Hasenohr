#include "event_loop.h"
#include "channel.h"
#include "poller.h"

#include <unistd.h>
#include <iostream>

#include <muduo/base/Logging.h>
#define MAXTIME int(10000)
thread_local event_loop* event_loop_in_thread=nullptr;
const int Max_time = MAXTIME;
event_loop::event_loop():looping(false),loop_thread_id(pthread_self()),quit_(false)
{
	//重复构造怎么办,需不需要终止构造
	assert(!event_loop_in_thread);
	event_loop_in_thread = this;
	poller_.reset(new poller(this));
}
event_loop::~event_loop()
{
	assert(!looping);
	event_loop_in_thread = nullptr;
}
void event_loop::loop()
{
	assert(!looping);
	assert_in_loop_thread();
	looping = true;
	while (!quit_)
	{
		//首先生成活动IO channel 列表
		active_channels.clear();
		muduo::Timestamp timestamp;
		timestamp=poller_->poll(Max_time, &active_channels);
		LOG_INFO<< timestamp.toFormattedString() << "\n";
		for (auto& item : active_channels)
		{
			item->handle_event();
		}
	}
	quit_ = false;
	looping = false;
}

void event_loop::assert_in_loop_thread()
{
	if (if_in_loop_thread())
	{
		//通过检查
	}
	else
	{
		abort_not_in_loop_thread();
	}
}

bool event_loop::if_in_loop_thread()
{
	return (static_cast<int>(pthread_self()) == loop_thread_id);
}

event_loop* event_loop::loop_in_thread()
{
	return event_loop_in_thread;
}

void event_loop::update_channel(channel* channel_)
{
	//在本线程中对本线程的channel进行更新
	assert(channel_->owner_loop() == this);
	assert_in_loop_thread();
	poller_->update_channel(channel_);
}

void event_loop::quit()
{
	quit_ = true;
}

void event_loop::abort_not_in_loop_thread()
{
	LOG_TRACE << "THE process already has an event loop ";
	assert(0);
}
 