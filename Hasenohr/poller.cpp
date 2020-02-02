#include "poller.h"
#include "channel.h"
#include <unistd.h>
#include <stdio.h>
#include <iostream>
poller::poller(event_loop* owner_loop) :owner_loop_(owner_loop)
{
}


muduo::Timestamp poller::poll(int time_out_ms, channel_list* active_channels)
{
	int active_num=::poll(pollfd_list_.data(), pollfd_list_.size(), time_out_ms);
	LOG_INFO << "poll active."<< active_num;
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
		channel* channel_in_map = it->second;
		assert(channel_in_map->fd() == channel_->fd());
		size_t index_ = channel_in_map->index();
		assert(index_ <= pollfd_list_.size());
		pollfd_list_[index_].events = short(channel_->events());
		pollfd_list_[index_].revents = 0;
		if (channel_->is_none_event())
			pollfd_list_[index_].fd = -pollfd_list_[index_].fd -1;
		else pollfd_list_[index_].fd = channel_->fd();
	}
	else
	{
		pollfd new_pollfd;
		new_pollfd.events = short(channel_->events());
		new_pollfd.revents = 0;
		new_pollfd.fd = channel_->fd();
		pollfd_list_.push_back(new_pollfd);
		channel_->set_index(int(pollfd_list_.size()) - 1);
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

void poller::remove_channel(channel* channel)
{
	owner_loop_->assert_in_loop_thread();
	assert(channel != nullptr);
	assert(channel_map_.find(channel->fd()) != channel_map_.end());
	int id_ = channel->index();
	if (id_ == int(pollfd_list_.size()-1))
	{
		pollfd_list_.erase(pollfd_list_.end()-1);
	}
	else
	{
		int final_fd_ = (pollfd_list_.end() - 1)->fd;
		if (final_fd_ < 0)
		{
			final_fd_ = -final_fd_ - 1;
		}
		std::iter_swap(pollfd_list_.end()-1,pollfd_list_.begin()+id_);
		//assert(channel_map_.find(final_fd_) != channel_map_.end());
		channel_map_[final_fd_]->set_index(id_);
		pollfd_list_.erase(pollfd_list_.end() - 1);
	}
	channel_map_.erase(channel->fd());
}




