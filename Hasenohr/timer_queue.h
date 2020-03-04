#pragma once
#include "timer.h"
#include "channel.h"

#include <vector>
#include <set>
#include <memory>

//timer_queueӦ�þ���һ��timer_set �������timer
class timer_queue
{
public:
	typedef std::shared_ptr<timer> timer_ptr;
	typedef std::weak_ptr<timer> timer_weak;
	typedef std::multiset<std::shared_ptr<timer>,less_compare> timer_set;
	timer_queue(event_loop* loop);
	//��Ӽ�ʱ��
	void add_timer_in_loop(const timer_ptr& timer_);
	void add_timer(const timer_ptr& timer_);
	//���ص��ڵļ�ʱ����(��ɾ�����ڵķ�ѭ����ʱ��)
	std::vector<timer_ptr> pop_front_queue();
	//ִ�е��ڵļ�ʱ���Ļص�,����ѭ����ʱ�����¼����б���
	void handle_alarm();
	//�ͷ��ļ���ʶ��
	~timer_queue();
	void set_timer_channel();
private:
	event_loop* owner_loop_;
	int time_fd_;
	channel timer_channel;
	timer_set timer_list_;
};