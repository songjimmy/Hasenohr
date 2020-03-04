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
	//�����ʱ���Ϊ��⵽��ȡʱ��ʱ��
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
	//��������ʲô�ĸ��£�
	void update();//
	
				  
	//���ݳ�Ա
	static const int k_none_event;
	static const int k_read_event;
	static const int k_write_event;
	//���ݳ�Ա �ļ���ʶ��
	const fd_t fd_;
	//���ĵ��¼������û��趨
	int events_;
	//���ڷ������¼�
	int revents_;
	//����ɶ:��¼channel��pollfd_vec�е�λ��
	int index_;
	//IO�¼����¼�������	
	Functor_with_timestamp func_read;
	Functor func_write;
	Functor func_erro;
	Functor func_close;
	event_loop* loop_;
	//channel��hanle_event�Ƿ����ڱ�����
	bool event_handling;
};