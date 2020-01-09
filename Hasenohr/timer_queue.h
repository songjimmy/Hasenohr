#pragma once
#include "timer.h"
#include "channel.h"

#include <vector>
#include <set>


//timer_queueӦ�þ���һ��timer_set �������timer
class timer_queue
{
public:
	typedef std::multiset<timer,less_compare> timer_set;
	timer_queue(event_loop* loop);
	//��Ӽ�ʱ��
	void add_timer(const timer& timer_);
	//���ص��ڵļ�ʱ����(��ɾ�����ڵķ�ѭ����ʱ��)
	std::vector<timer> pop_front_queue();
	//ִ�е��ڵļ�ʱ���Ļص�,����ѭ����ʱ�����¼����б���
	void handle_alarm();
	//�ͷ��ļ���ʶ��
	~timer_queue();
	void set_timer_channel();
private:
	timer_set timer_list_;
	channel timer_channel;
	//���ܴ���й¶����Դ
	int time_fd_;
	//��ʱ����
	event_loop* owner_loop_;
};