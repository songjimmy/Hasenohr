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
	Functor func_read;
	Functor func_write;
	Functor func_erro;
	event_loop* loop_;
};