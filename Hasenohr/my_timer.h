#pragma once
#include <sys/timerfd.h>
#include <memory>
#include <functional>
#include "channel.h"
//��ʱ��
//�����ļ��������
//������ʱ���¼�ʱ��event loop�ᴥ����Ӧ�¼�
//��Ҫ����timerfd��һ���ļ���poll�ı�����ʽ�Ĳ�ͬ
//timer�ڴ�����Ӧ�����٣���time__queue�������

class event_loop;
class my_timer
{
public:
	typedef int time_fd;
	typedef std::function<void(void)> callback;
	//һЩ����
	my_timer(int time_ms, event_loop* owner_loop);
	//����һ�����������ʱ����channel
	~my_timer();
	void set_time_callback(const callback& cb);
private:
	//�¼��ļ���ʶ��,Ӧ�����Զ����ɵ�
	//ָ���ļ�����Դ������������ʱӦ���ͷ�
	//���Ǽ򵥵�����������ʱ��ֻ�ܴ���ʱ��ѭ��
	time_fd time_fd_;
	//��ʱ����
	itimerspec howlong;
	//��Ϊһ����ʱ��������һ���ļ���д��ʵ�ֵģ�һ���ļ���д���Ĺ�����һ��channelʵ�֣�����timer�з�װһ��channel�Ǻ����
	//�������������ʱ����channel
	channel channel_;
	event_loop* owner_loop_;
};