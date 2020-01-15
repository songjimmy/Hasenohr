#pragma once
#include "channel.h"
class eventfd_channel
{
public:
	typedef int fd_t;
	eventfd_channel(event_loop* loop);
	~eventfd_channel();
	//向自身的fd_t中进行写入
	void wake_up();
private:
	const fd_t eventfd_;
	channel channel_;
};