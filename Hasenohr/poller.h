#pragma once
#include <vector>
#include <map>
#include <poll.h>
#include <sys/epoll.h>
#include <muduo/base/Timestamp.h>
#include <muduo/base/Logging.h>
#include "event_loop.h"

class channel;
class poller
{
public:
	typedef std::vector<channel*> channel_list;
	//���캯��������һ��channel_list����ת��Ϊchannel_map��Ҳ��ͬʱ���event��revent__���ܲ���Ҫ ��Ϊ��ʼ��ʱΪ��
	//ͬʱ����event_loop* ��poller��������loop��
	poller(event_loop* owner_loop__);
	//���ɿ���
	poller(const poller&) = delete;
	//ȱʡ��������
	//��ѯ������Ӧ����һ��ʱ���,��active_channels��ͷ�Ӷ���
	muduo::Timestamp poll(int time_out_ms,channel_list* active_channels);
	//���Է�������ֹ���̵߳�poller����ʹ��
	void assert_in_loop_thread();
	//���£�����
	void update_channel(channel* channel_);
	void remove_channel(channel* channel_);
private:
	enum index_state { init = -1,out,in };
	typedef std::map<int, channel*> channel_map;
	typedef std::vector<epoll_event> event_list;
	
	//���ݻ��channels������ѯ�channel,��֪ͨchannel������ʵ���¼�״̬
	void fill_active_channels(channel_list* active_channels,int active_num);

	//���²���ֻ��epfd���в���
	void insert_event(channel* channel_);
	void modify_event(channel* channel_);
	void delete_event(channel* channel_);
	
	//eoll�������ļ���
	int epfd;
	//������ѯ�ض�fd_t��Ӧ��channel,�Ӷ�֪ͨchannel������صĲ���
	channel_map channel_map_;
	//
	event_list event_list_;
	event_list active_events_;
	//������event_loop��ָ��
	event_loop* owner_loop_;
};
