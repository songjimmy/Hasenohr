#pragma once
#include <vector>
#include <map>
#include <poll.h>
#include <muduo/base/Timestamp.h>
#include <muduo/base/Logging.h>
#include "event_loop.h"

class channel;
struct pollfd;
class poller
{
public:
	typedef int fd_t;
	typedef std::vector<channel*> channel_list;
	//���캯��������һ��channel_list����ת��Ϊchannel_map��Ҳ��ͬʱ���event��revent__���ܲ���Ҫ ��Ϊ��ʼ��ʱΪ��
	//ͬʱ����event_loop* ��poller��������loop��
	poller(event_loop* owner_loop);
	//���ɿ���
	poller(const poller&) = delete;
	//ȱʡ��������
	//��ѯ������Ӧ����һ��ʱ���,��active_channels��ͷ�Ӷ���
	muduo::Timestamp poll(int time_out_ms,channel_list* active_channels);
	//���Է�������ֹ���̵߳�poller����ʹ��
	void assert_in_loop_thread();
	//���£�����
	void update_channel(channel* channel_);
private:
	//���ݻ��channels������ѯ�channel,��֪ͨchannel
	void fill_active_channels(channel_list* active_channels,int num_active);

	typedef std::vector<pollfd> pollfd_list;
	typedef std::map<fd_t, channel*> channel_map;
	//pollfd�����飬����ά��poll��������Ҫ��pollfd����
	pollfd_list pollfd_list_;
	//������ѯ�ض�fd_t��Ӧ��channel,�Ӷ�֪ͨchannel������صĲ���
	channel_map channel_map_;
	//������event_loop��ָ��
	event_loop* owner_loop_;
};
