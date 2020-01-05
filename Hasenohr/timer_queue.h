#pragma once
#include "timer.h"
#include "channel.h"

#include <vector>
#include <set>


//timer_queueӦ�þ���һ��timer_set �������timer
class timer_queue
{
public:
	typedef std::set<timer,less_compare> timer_set;
	timer_queue(event_loop* loop);
	//��Ӽ�ʱ��
	void add_timer_queue(const timer& timer_);
	//���ص��ڵļ�ʱ������ɾ�����ڵķ�ѭ����ʱ��
	std::vector<timer> pop_front_queue();
	//ִ�е��ڵļ�ʱ���Ļص�
	void handle_alarm();
	~timer_queue();
private:
	timer_set timer_list_;
	channel timer_channel;
	int time_fd_;
	//��ʱ����
	itimerspec howlong;
	event_loop* owner_loop_;
};