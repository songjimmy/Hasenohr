#include "channel.h"
#include <unistd.h>
#include <poll.h>
#include"event_loop.h"
const int channel::k_none_event = 0;//等于0是什么语义？
//紧急优先普通可读
const int channel::k_read_event=POLLIN | POLLPRI;
//可写
const int channel::k_write_event = POLLOUT;

channel::channel(event_loop* loop, fd_t fd)
	:loop_(loop),fd_(fd),events_(0),revents_(0),index_(-1)
{}
//事件分发函数
void channel::handle_event()
{
	//有错误发生或非法的事件描述符
	if (events_ & (POLLNVAL | POLLERR))
	{
		if (func_erro) func_erro();
	}
	//读
	if (events_ & k_read_event)
	{
		if (func_read) func_read();
	}
	//写
	if (events_&k_write_event)
	{
		if(func_write) func_write();
	}
	//可扩展
}

void channel::set_read_callback(const Functor& cb)
{
	func_read = cb;
}

void channel::set_write_callback(const Functor&cb)
{
	func_write = cb;
}

void channel::set_erro_callback(const Functor& cb)
{
	func_erro = cb;
}

int channel::fd() const
{
	return fd_;
}

int channel::events() const
{
	return events_;
}

int channel::revents() const
{
	return revents_;
}

event_loop* channel::owner_loop() const
{
	return loop_;
}

void channel::set_revents(int revents)
{
	revents_ = revents;
}

bool channel::is_none_event() const
{
	return revents_==k_none_event;
}
//使channel能关注“可读”事件
void channel::enable_reading()
{
	events_ |= k_read_event ;//使读掩码置1而不影响其他位
	update();
}

int channel::index() const
{
	return index_;
}

void channel::set_index(int index_addr)
{
	index_ = index_addr;
}

void channel::update()
{
	loop_->update_channel(this);
}


