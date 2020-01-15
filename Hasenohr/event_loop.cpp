#include "event_loop.h"
#include "channel.h"
#include "poller.h"
#include "eventfd_channel.h"

#include <unistd.h>
#include <sys/eventfd.h>

#include <muduo/base/Logging.h>
#define MAXTIME int(10000)
thread_local event_loop* event_loop_in_thread=nullptr;
const int Max_time = MAXTIME;
event_loop::event_loop()
:looping(false),
loop_thread_id(pthread_self()),
quit_(false),
timer_queue_(nullptr),
eventfd_channel_(nullptr)
{
	//�ظ�������ô��,�費��Ҫ��ֹ����
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
		//�������ɻIO channel �б�
		active_channels.clear();
		muduo::Timestamp timestamp;
		timestamp=poller_->poll(Max_time, &active_channels);
		LOG_INFO<< timestamp.toFormattedString() << "\n";
		for (auto& item : active_channels)
		{
			item->handle_event();
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
		//ͨ�����
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
	//�ڱ��߳��жԱ��̵߳�channel���и���
	assert(channel_->owner_loop() == this);
	assert_in_loop_thread();
	poller_->update_channel(channel_);
}

void event_loop::quit()
{
	//������߳��ڵ���quit�����߳��������¼��ص�����pending_functors�е���quit������������poll
	quit_ = true;
	//������߳������quit,�̴߳�ʱ������quit��,wake_up���Կ��ٽ�������poll
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
	//�ú�������Ŀɵ��ö���ϣ���ܹ��������ã���ˣ�����Ǵ������̴߳������������ִ�е�pending_functors���룬��Ҫ���»���
	if (!if_in_loop_thread()|calling_pending_functors)
	{
		wake_up();
	}
}

void event_loop::do_pending_functors()
{
	//��ĳЩ����Ŀɵ��ö������queue_in_loop����ʱ��֪ͨqueue_in_loop��ִ�����ʱ����LOOP
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