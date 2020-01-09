#pragma once
#include "timer.h"
#include "channel.h"

#include <vector>
#include <set>


//timer_queue应该具有一个timer_set 可以添加timer
class timer_queue
{
public:
	typedef std::multiset<timer,less_compare> timer_set;
	timer_queue(event_loop* loop);
	//添加计时器
	void add_timer(const timer& timer_);
	//返回到期的计时器，(并删除到期的非循环计时器)
	std::vector<timer> pop_front_queue();
	//执行到期的计时器的回调,并将循环计时器重新加入列表中
	void handle_alarm();
	//释放文件标识符
	~timer_queue();
	void set_timer_channel();
private:
	timer_set timer_list_;
	channel timer_channel;
	//可能存在泄露的资源
	int time_fd_;
	//定时设置
	event_loop* owner_loop_;
};