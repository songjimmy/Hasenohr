#include "event_loop_thread.h"

event_loop_thread::event_loop_thread()
	:thread_(std::bind(&event_loop_thread::thread_functor, this)),
	cond(mutex_)
{	
}

event_loop* event_loop_thread::loop()
{
	//检查线程未启动
	assert(!thread_.started());
	thread_.start();
	{
		muduo::MutexLockGuard lock(mutex_);
		while (!loop_)
		{
			cond.wait();
		}
	}
	return loop_;
}

void event_loop_thread::thread_functor()
{
	event_loop event_loop_;
	{
		muduo::MutexLockGuard lock(mutex_);
		loop_ = &event_loop_;
		cond.notify();
	}
	event_loop_.loop();
	assert(false);
	printf("shouldn't reach");
	printf("shouldn't reach");
}
//这里条件变量的用意 在LOOP构造完成后 才通知loop开始正常工作
