#pragma once
#include <pthread.h>
#include <assert.h>

#include <functional>
#include <memory>

#include <muduo/base/Logging.h>
#include <muduo/base/Timestamp.h>
#include <muduo/base/Mutex.h>

#include <vector>

#include "timer_queue.h"
#include "eventfd_channel.h"
class channel;
class poller;
class event_loop
{
public:
	typedef std::vector<channel*> channel_list;
	//可以绑定任意谓语
	typedef std::function<void(void)> Functor;
	typedef std::vector<Functor> pending_functors;
	event_loop();
	event_loop(const event_loop&) = delete;
	~event_loop();
	void loop();
	//线程不安全方法的检查,检查不通过会调用abort
	void assert_in_loop_thread();
	bool if_in_loop_thread();
	//静态方法，检查线程是否具有一个IO事件循环,如果有 返回其指针
	static event_loop* loop_in_thread();
	void update_channel(channel*);
	void quit();
	void run_at(Functor call_back,muduo::Timestamp time_stamp);
	void run_every(Functor call_back, double interval_second);
	void run_after(Functor call_back,double time_second);
	//添加线程间通信传来的可调用对象，唤醒后执行
	void run_in_loop(const Functor&);
	void queue_in_loop(const Functor&);
	//唤醒线程
	void wake_up();
	//在唤醒后进行执行
	void do_pending_functors();
	void remove_channel(channel* channel);
private:
	//线程检查不通过的处理
	void abort_not_in_loop_thread();
	//是否正在循环
	bool looping;
	//loop所在线程编号
	const pid_t loop_thread_id;
	//循环是否退出的标志位
	bool quit_;
	//轮询器
	std::unique_ptr<poller> poller_;
	//避免重复构造销毁
	channel_list active_channels;
	//线程间通信手段
	std::unique_ptr<eventfd_channel> eventfd_channel_;
	//事件循环附带一个计时器功能
	std::unique_ptr<timer_queue> timer_queue_;
	//未执行的唤醒动作队列
	pending_functors pending_functors_;
	//检查pending_functors是否被调用的标识
	bool calling_pending_functors;
	//保护可调用对象序列的锁
	muduo::MutexLock mutex_;
};