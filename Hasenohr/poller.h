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
	//构造函数，传入一个channel_list用来转化为channel_map，也能同时获得event和revent__可能不需要 因为初始化时为空
	//同时传入event_loop* 将poller和其所属loop绑定
	poller(event_loop* owner_loop__);
	//不可拷贝
	poller(const poller&) = delete;
	//缺省析构函数
	//轮询方法，应返回一个时间戳,向active_channels后头加东西
	muduo::Timestamp poll(int time_out_ms,channel_list* active_channels);
	//断言方法，防止跨线程的poller方法使用
	void assert_in_loop_thread();
	//更新？？？
	void update_channel(channel* channel_);
	void remove_channel(channel* channel_);
private:
	enum index_state { init = -1,out,in };
	typedef std::map<int, channel*> channel_map;
	typedef std::vector<epoll_event> event_list;
	
	//根据活动的channels数量查询活动channel,并通知channel更新其实际事件状态
	void fill_active_channels(channel_list* active_channels,int active_num);

	//以下操作只对epfd进行操作
	void insert_event(channel* channel_);
	void modify_event(channel* channel_);
	void delete_event(channel* channel_);
	
	//eoll的特殊文件符
	int epfd;
	//用来查询特定fd_t对应的channel,从而通知channel进行相关的操作
	channel_map channel_map_;
	//
	event_list event_list_;
	event_list active_events_;
	//所属的event_loop的指针
	event_loop* owner_loop_;
};
