#include "event_loop.h"
#include "channel.h"
#include "poller.h"
#include "eventfd_channel.h"

#include <unistd.h>
#include <sys/eventfd.h>
#include <signal.h>

#include <muduo/base/Logging.h>
#define MAXTIME int(10000)
thread_local event_loop* event_loop_in_thread=nullptr;
const int Max_time = MAXTIME;
//使用全局变量忽略sigpipe
class ignore_sigpipe
{
public:
	ignore_sigpipe()
	{
		::signal(SIGPIPE, SIG_IGN);
	}
};
ignore_sigpipe initObj;
event_loop::event_loop()
:looping(false),
quit_(false),
loop_thread_id(pid_t(pthread_self()))
{
	//重复构造怎么办,需不需要终止构造
	assert(!event_loop_in_thread);
	event_loop_in_thread = this;
	poller_.reset(new poller(this));
	eventfd_channel_.reset(new eventfd_channel(this));
	timer_queue_.reset(new timer_queue(this));
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
		LOG_INFO<< timestamp.toFormattedString() ;
		for (auto& item : active_channels)
		{
			item->handle_event(timestamp);
		}
		do_pending_functors();
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
	//如果在线程内调用quit，？线程总是在事件回调或者pending_functors中调用quit，不会阻塞在poll
	quit_ = true;
	//如果在线程外调用quit,线程此时阻塞在quit处,wake_up可以快速结束本次poll
	if (!if_in_loop_thread())
	{
		wake_up();
	}
}

void event_loop::abort_not_in_loop_thread()
{
	LOG_TRACE << "THE process already has an event loop ";
	assert(0);
}

void event_loop::run_in_loop(const Functor& cb)
{
	if (if_in_loop_thread())
	{
		cb();
	}
	else
	{
		queue_in_loop(cb);
	}
}

void event_loop::queue_in_loop(const Functor& cb)
{
	{
		muduo::MutexLockGuard lock(mutex_);
		pending_functors_.push_back(cb);
	}
	//该函数传入的可调用对象希望能够立即调用，因此，如果是从其他线程传入或者由正在执行的pending_functors传入，需要重新唤醒
	if (!if_in_loop_thread()|calling_pending_functors)
	{
		wake_up();
	}
}

void event_loop::do_pending_functors()
{
	//当某些传入的可调用对象包括queue_in_loop操作时，通知queue_in_loop在执行完毕时唤醒LOOP
	calling_pending_functors = true;
	pending_functors calling_functors;
	{
		muduo::MutexLockGuard lock(mutex_);
		calling_functors.swap(pending_functors_);
	}
	for (auto& item : calling_functors)
	{
		item();
	}
	calling_pending_functors = false;
}

void event_loop::remove_channel(channel* channel)
{
	poller_->remove_channel(channel);
}
