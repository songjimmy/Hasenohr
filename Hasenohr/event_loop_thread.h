#pragma once
#include "event_loop.h"

#include <muduo/base/Thread.h>
#include <muduo/base/Condition.h>
#include <muduo/base/Mutex.h>

class event_loop_thread 
{
public:
	typedef std::function<void(void)> Functor;
	event_loop_thread();
	event_loop_thread(const event_loop_thread&)=delete;
	event_loop_thread operator=(const event_loop_thread&)=delete;
	event_loop* loop();
	void thread_functor();
private:
	muduo::Thread thread_;
	event_loop*  loop_;
	muduo::MutexLock mutex_;
	muduo::Condition cond;
};