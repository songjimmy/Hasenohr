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
	//���԰�����ν��
	typedef std::function<void(void)> Functor;
	typedef std::vector<Functor> pending_functors;
	event_loop();
	event_loop(const event_loop&) = delete;
	~event_loop();
	void loop();
	//�̲߳���ȫ�����ļ��,��鲻ͨ�������abort
	void assert_in_loop_thread();
	bool if_in_loop_thread();
	//��̬����������߳��Ƿ����һ��IO�¼�ѭ��,����� ������ָ��
	static event_loop* loop_in_thread();
	void update_channel(channel*);
	void quit();
	void run_at(Functor call_back,muduo::Timestamp time_stamp);
	void run_every(Functor call_back, double interval_second);
	void run_after(Functor call_back,double time_second);
	//����̼߳�ͨ�Ŵ����Ŀɵ��ö��󣬻��Ѻ�ִ��
	void run_in_loop(const Functor&);
	void queue_in_loop(const Functor&);
	//�����߳�
	void wake_up();
	//�ڻ��Ѻ����ִ��
	void do_pending_functors();
	void remove_channel(channel* channel);
private:
	//�̼߳�鲻ͨ���Ĵ���
	void abort_not_in_loop_thread();
	//�Ƿ�����ѭ��
	bool looping;
	//loop�����̱߳��
	const pid_t loop_thread_id;
	//ѭ���Ƿ��˳��ı�־λ
	bool quit_;
	//��ѯ��
	std::unique_ptr<poller> poller_;
	//�����ظ���������
	channel_list active_channels;
	//�̼߳�ͨ���ֶ�
	std::unique_ptr<eventfd_channel> eventfd_channel_;
	//�¼�ѭ������һ����ʱ������
	std::unique_ptr<timer_queue> timer_queue_;
	//δִ�еĻ��Ѷ�������
	pending_functors pending_functors_;
	//���pending_functors�Ƿ񱻵��õı�ʶ
	bool calling_pending_functors;
	//�����ɵ��ö������е���
	muduo::MutexLock mutex_;
};