#include <sys/eventfd.h>

#include <muduo/base/Logging.h>

#include "eventfd_channel.h"
//线程间通信手段
void eventfd_read_callback(int fd)
{
	LOG_INFO << "eventfd is readable.";
	uint64_t buf;
	ssize_t n=read(fd, &buf, sizeof buf);
	if (n != sizeof buf)
	{
		LOG_ERROR << "wrong read";
	}
}

eventfd_channel::eventfd_channel(event_loop* loop)
	:eventfd_(eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)),
	channel_(loop, eventfd_)
{
	channel_.set_read_callback(std::bind(&eventfd_read_callback,eventfd_));
	channel_.enable_reading();
}

eventfd_channel::~eventfd_channel()
{
	close(eventfd_);
}

void eventfd_channel::wake_up()
{
	uint64_t sign=1;
	ssize_t n=write(eventfd_, &sign, sizeof sign);
	if (n != sizeof sign)
	{
		LOG_ERROR << "wrong write";
	}
}
