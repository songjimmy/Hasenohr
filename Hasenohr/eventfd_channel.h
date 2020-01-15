#pragma once
#include "channel.h"
class eventfd_channel
{
public:
	typedef int fd_t;
	eventfd_channel(event_loop* loop);
	~eventfd_channel();
	//�������fd_t�н���д��
	void wake_up();
private:
	const fd_t eventfd_;
	channel channel_;
};