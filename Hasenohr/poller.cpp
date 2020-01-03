#include "poller.h"
#include "channel.h"
#include <unistd.h>
poller::poller(event_loop* owner_loop) :owner_loop_(owner_loop)
{
}


muduo::Timestamp poller::poll(int time_out_ms, channel_list* active_channels)
{
	int active_num=::poll(pollfd_list_.data(), pollfd_list_.size(), time_out_ms);
	muduo::Timestamp now_ = muduo::Timestamp::now();
	if (active_num < 0)
	{
		LOG_ERROR << "\nwrong poll\n";
	}
	else if (active_num ==0)
	{
		LOG_TRACE << "\nnoting happend\n";
	}
	else
	{
		fill_active_channels(active_channels, active_num);
	}
	return now_;
}


void poller::assert_in_loop_thread()
{
	owner_loop_->assert_in_loop_thread();
}
//不在channel_map中的和在其中的


void poller::update_channel(channel* channel_)
{
	channel_map_.size();
	auto it = channel_map_.end();
	if(!channel_map_.empty())
		it = channel_map_.find(channel_->fd());
	//在map中找到了
	if ( it!= channel_map_.end())
	{
		channel* channel_in_map = it->second;//channel*
		assert(channel_in_map->fd() == channel_->fd());
		int index_ = channel_in_map->index();
		assert(index_ <= pollfd_list_.size());
		pollfd_list_[index_].events = channel_->events();
		pollfd_list_[index_].revents = 0;
		if (channel_->is_none_event())
			pollfd_list_[index_].events = -1;
	}
	else
	{
		pollfd new_pollfd;
		new_pollfd.events = channel_->events();
		new_pollfd.revents = 0;
		new_pollfd.fd = channel_->fd();
		pollfd_list_.push_back(new_pollfd);
		channel_->set_index(pollfd_list_.size() - 1);
		channel_map_[channel_->fd()] = channel_;
	}
}

void poller::fill_active_channels(channel_list* active_channels, int num_active)
{
	for (auto& pollfd_it:pollfd_list_)
	{
		if (num_active>0&&pollfd_it.revents>0)
		{
			--num_active;
			auto it = channel_map_.find(pollfd_it.fd);
			assert(it != channel_map_.end());
			channel* active_channel = it->second;
			assert(active_channel->fd() == pollfd_it.fd);
			//更新了激活事件所属channel的实际事件，为下一步event_handle做准备
			active_channel->set_revents(pollfd_it.revents);
			active_channels->push_back(active_channel);
		}
	}
}




