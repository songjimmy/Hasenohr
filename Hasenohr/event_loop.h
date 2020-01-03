#pragma once
#include <pthread.h>
#include <assert.h>

#include <functional>
#include <memory>

#include <muduo/base/Logging.h>
#include <vector>
class channel;
class poller;
class event_loop
{
public:
	typedef std::vector<channel*> channel_list;
	//可以绑定任意谓语
	typedef std::function<void(void)> Functor;
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
private:
	//线程检查不通过的处理
	void abort_not_in_loop_thread();
	//是否正在循环
	bool looping;
	//loop所在线程编号
	const pid_t loop_thread_id;
	//循环是否退出的标志位
	bool quit_;
	std::unique_ptr<poller> poller_;
	channel_list active_channels;
};