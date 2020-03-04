#pragma once
#include "timer.h"
#include "channel.h"

#include <vector>
#include <set>
#include <memory>

//timer_queue应该具有一个timer_set 可以添加timer
class timer_queue
{
public:
	typedef std::shared_ptr<timer> timer_ptr;
	typedef std::weak_ptr<timer> timer_weak;
	typedef std::multiset<std::shared_ptr<timer>,less_compare> timer_set;
	timer_queue(event_loop* loop);
	//添加计时器
	void add_timer_in_loop(const timer_ptr& timer_);
	void add_timer(const timer_ptr& timer_);
	//返回到期的计时器，(并删除到期的非循环计时器)
	std::vector<timer_ptr> pop_front_queue();
	//执行到期的计时器的回调,并将循环计时器重新加入列表中
	void handle_alarm();
	//释放文件标识符
	~timer_queue();
	void set_timer_channel();
private:
	event_loop* owner_loop_;
	int time_fd_;
	channel timer_channel;
	timer_set timer_list_;
};