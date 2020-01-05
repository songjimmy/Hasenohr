#pragma once
#include<stdio.h>
#include<memory>
#include<functional>
class event_loop;
class channel
{
public:
	typedef int fd_t;
	typedef std::function<void(void)> Functor;
	//event_loop?
	channel(event_loop*,fd_t);
	channel(const channel&) = delete;
	//channel(channel&&);
	void handle_event();
	void set_read_callback(const Functor&);
	void set_write_callback(const Functor&);
	void set_erro_callback(const Functor&);
	int fd() const;
	int events() const;
	int revents() const;
	event_loop* owner_loop() const;
	void set_revents(int);
	bool is_none_event() const;
	void enable_reading();
	void unenable_reading();
	int index() const;
	void set_index(int index_addr);
private:
	//用来进行什么的更新？
	void update();//
	
				  
	//数据成员
	static const int k_none_event;
	static const int k_read_event;
	static const int k_write_event;
	//数据成员 文件标识符
	const fd_t fd_;
	//关心的事件，由用户设定
	int events_;
	//正在发生的事件
	int revents_;
	//这是啥:记录channel在pollfd_vec中的位置
	int index_;
	//IO事件的事件处理函数	
	Functor func_read;
	Functor func_write;
	Functor func_erro;
	event_loop* loop_;
};