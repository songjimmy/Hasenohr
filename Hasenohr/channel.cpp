#include "channel.h"
#include <unistd.h>
#include <poll.h>
#include "event_loop.h"
#include <iostream>
const int channel::k_none_event = 0;//等于0是什么语义？
//紧急优先普通可读
const int channel::k_read_event=POLLIN | POLLPRI;
//可写
const int channel::k_write_event = POLLOUT;

channel::channel(event_loop* loop, fd_t fd)
	:fd_(fd),events_(0),revents_(0),index_(-1),loop_(loop), event_handling(false)
{
}
channel::~channel()
{
	//channel析构时不应当在执行handle_event
	assert(!event_handling);
	LOG_INFO << "channel is ~";
}
//事件分发函数
void channel::handle_event()
{
	event_handling=true;
	//当对面挂起，且套接字不可读时，应当关闭
	if ((revents_ & POLLHUP) && (revents_ & !POLLIN))
	{
		LOG_INFO << "connection will be closed";
		if (func_close) func_close();//把删除这个channel的操作queue_in_loop..queue_in_loop的动作会在一次循环的最后执行
	}
	if (revents_&POLLNVAL)
	{
		LOG_INFO << "POLLNVAL";
	}
	//有错误发生或非法的事件描述符
	if (revents_ & (POLLNVAL | POLLERR))
	{
		if (func_erro) func_erro();
	}
	//读
	if (revents_ & (k_read_event|POLLRDHUP))
	{
		if (func_read) func_read();
	}
	//写
	if (revents_&k_write_event)
	{
		if(func_write) func_write();
	}
	//可扩展
	event_handling = false;
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

void channel::set_close_callback(const Functor& func_close_)
{
	func_close = func_close_;
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
	return events_==k_none_event;
}
//使channel能关注“可读”事件
void channel::enable_reading()
{
	events_ |= k_read_event ;//使读掩码置1而不影响其他位
	update();
}

void channel::unenable_reading()
{
	events_ = k_none_event;//使读掩码置0
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

void channel::print()
{
	LOG_INFO <<"events:" <<events_;
	LOG_INFO <<"revents:" <<revents_;
}

void channel::update()
{
	loop_->update_channel(this);
}


