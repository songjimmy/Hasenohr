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
	//���԰�����ν��
	typedef std::function<void(void)> Functor;
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
private:
	//�̼߳�鲻ͨ���Ĵ���
	void abort_not_in_loop_thread();
	//�Ƿ�����ѭ��
	bool looping;
	//loop�����̱߳��
	const pid_t loop_thread_id;
	//ѭ���Ƿ��˳��ı�־λ
	bool quit_;
	std::unique_ptr<poller> poller_;
	channel_list active_channels;
};