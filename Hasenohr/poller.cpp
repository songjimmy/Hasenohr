#include "poller.h"

poller::poller(event_loop* owner_loop__)
{
	owner_loop_ = owner_loop__;
	epfd = ::epoll_create(1024);
}

muduo::Timestamp poller::poll(int time_out_ms, channel_list* active_channels)
{
	assert_in_loop_thread();
	active_events_.clear();
	active_events_.resize(channel_map_.size());
	int active_num = ::epoll_wait(epfd,active_events_.data(),int(active_events_.size()), time_out_ms);
	muduo::Timestamp now_ = muduo::Timestamp::now();
	if (active_num < 0)
	{
		LOG_ERROR << "wrong poll";
		LOG_ERROR << errno;
	}
	else if (0==active_num)
	{
		LOG_TRACE << "noting happend\n";
	}
	else
	{
		fill_active_channels(active_channels,active_num);
	}
	return now_;
}

void poller::assert_in_loop_thread()
{
	owner_loop_->assert_in_loop_thread();
}

void poller::update_channel(channel* channel_)
{
	int fd = channel_->fd();
	//Ìí¼Ó
	if (channel_->index()==out|| channel_->index()==init)
	{
		assert((channel_->index() == init&&channel_map_.find(fd) == channel_map_.end())|| (channel_->index() == out && channel_map_.find(fd) != channel_map_.end()));
		insert_event(channel_);
		channel_map_.insert({ fd,channel_ });
		channel_->set_index(in);
	}
	//ÐÞ¸Ä
	else
	{
		assert(channel_map_.find(fd) != channel_map_.end());
		assert(channel_map_[fd] == channel_);
		if (channel_->is_none_event())
		{
			delete_event(channel_);
			channel_->set_index(out);
		}
		else
		{
			modify_event(channel_);
		}
	}
}

void poller::remove_channel(channel* channel_)
{
	int fd = channel_->fd();
	assert(channel_map_.find(fd) != channel_map_.end());
	assert(channel_map_[fd] == channel_);
	if (channel_->index() == in)
	{
		delete_event(channel_);
	}
	else
	{
		assert(channel_->index() == out);
	}
	channel_map_.erase(channel_map_.find(fd));
}

void poller::fill_active_channels(channel_list* active_channels,int active_num)
{
	for (int i=0;i<active_num;++i)
	{
		epoll_event& event_ = active_events_[i];
		channel* active_channel_ = (channel*)event_.data.ptr;
		active_channel_->set_revents(event_.events);
		active_channels->push_back(active_channel_);
	}
}

void poller::insert_event(channel* channel_)
{
	epoll_event epoll_event_;
	bzero(&epoll_event_, sizeof epoll_event_);
	epoll_event_.data.ptr = channel_;
	epoll_event_.events = channel_->events();
	if (::epoll_ctl(epfd, EPOLL_CTL_ADD, channel_->fd(), &epoll_event_) < 0)
	{
		LOG_ERROR << "poller::insert_event at fd="<<channel_->fd();
	}
}

void poller::modify_event(channel* channel_)
{
	epoll_event epoll_event_;
	bzero(&epoll_event_, sizeof epoll_event_);
	epoll_event_.data.ptr = channel_;
	epoll_event_.events = channel_->events();
	if (::epoll_ctl(epfd, EPOLL_CTL_MOD, channel_->fd(), &epoll_event_) < 0)
	{
		LOG_ERROR << "poller::modify_event at fd=" << channel_->fd();
	}
}

void poller::delete_event(channel* channel_)
{
	epoll_event epoll_event_;
	bzero(&epoll_event_, sizeof epoll_event_);
	epoll_event_.data.ptr = channel_;
	epoll_event_.events = channel_->events();
	if (::epoll_ctl(epfd, EPOLL_CTL_DEL, channel_->fd(), &epoll_event_))
	{
		LOG_ERROR << "poller::delete_event at fd=" << channel_->fd();
	}
}