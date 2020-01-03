#include "channel.h"
#include <unistd.h>
#include <poll.h>
#include"event_loop.h"
const int channel::k_none_event = 0;//����0��ʲô���壿
//����������ͨ�ɶ�
const int channel::k_read_event=POLLIN | POLLPRI;
//��д
const int channel::k_write_event = POLLOUT;

channel::channel(event_loop* loop, fd_t fd)
	:loop_(loop),fd_(fd),events_(0),revents_(0),index_(-1)
{}
//�¼��ַ�����
void channel::handle_event()
{
	//�д�������Ƿ����¼�������
	if (events_ & (POLLNVAL | POLLERR))
	{
		if (func_erro) func_erro();
	}
	//��
	if (events_ & k_read_event)
	{
		if (func_read) func_read();
	}
	//д
	if (events_&k_write_event)
	{
		if(func_write) func_write();
	}
	//����չ
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
//ʹchannel�ܹ�ע���ɶ����¼�
void channel::enable_reading()
{
	events_ |= k_read_event ;//ʹ��������1����Ӱ������λ
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


