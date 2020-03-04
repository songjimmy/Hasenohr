#pragma once
#include<stdio.h>
#include<memory>
#include<functional>
#include<muduo/base/Timestamp.h>
class event_loop;
class channel
{
public:
	typedef int fd_t;
	typedef std::function<void(void)> Functor;
	//输入的时间戳为检测到读取时的时间
	typedef std::function<void(muduo::Timestamp)> Functor_with_timestamp;
	//event_loop?
	channel(event_loop*,fd_t);
	channel(const channel&) = delete;
	//channel(channel&&);
	~channel();
	void handle_event(muduo::Timestamp return_time);
	void set_read_callback(const Functor_with_timestamp&);
	void set_write_callback(const Functor&);
	void set_erro_callback(const Functor&);
	void set_close_callback(const Functor&);
	int fd() const;
	int events() const;
	int revents() const;
	event_loop* owner_loop() const;
	void set_revents(int);
	bool is_none_event() const;
	void enable_reading();
	void unenable_all();
	bool is_writing() const;
	void enable_writing();
	void unenable_writing();
	int index() const;
	void set_index(int index_addr);
	void print();
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
	Functor_with_timestamp func_read;
	Functor func_write;
	Functor func_erro;
	Functor func_close;
	event_loop* loop_;
	//channel的hanle_event是否正在被调用
	bool event_handling;
};