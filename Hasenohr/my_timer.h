#pragma once
#include <sys/timerfd.h>
#include <memory>
#include <functional>
#include "channel.h"
//计时器
//基于文件符的设计
//当发生时间事件时，event loop会触发相应事件
//需要考虑timerfd和一般文件符poll的表现形式的不同
//timer在触发后应当销毁，由time__queue负责管理

class event_loop;
class my_timer
{
public:
	typedef int time_fd;
	typedef std::function<void(void)> callback;
	//一些参数
	my_timer(int time_ms, event_loop* owner_loop);
	//生成一个管理这个定时器的channel
	~my_timer();
	void set_time_callback(const callback& cb);
private:
	//事件文件标识符,应当是自动生成的
	//指向文件符资源的量，在析构时应当释放
	//考虑简单的情况，这个计时器只能处理定时非循环
	time_fd time_fd_;
	//定时设置
	itimerspec howlong;
	//因为一个计时器就是用一个文件读写符实现的，一个文件读写符的管理由一个channel实现，所以timer中封装一个channel是合理的
	//用来管理这个计时器的channel
	channel channel_;
	event_loop* owner_loop_;
};