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
	//构造函数，传入一个channel_list用来转化为channel_map，也能同时获得event和revent__可能不需要 因为初始化时为空
	//同时传入event_loop* 将poller和其所属loop绑定
	poller(event_loop* owner_loop);
	//不可拷贝
	poller(const poller&) = delete;
	//缺省析构函数
	//轮询方法，应返回一个时间戳,向active_channels后头加东西
	muduo::Timestamp poll(int time_out_ms,channel_list* active_channels);
	//断言方法，防止跨线程的poller方法使用
	void assert_in_loop_thread();
	//更新？？？
	void update_channel(channel* channel_);
private:
	//根据活动的channels数量查询活动channel,并通知channel
	void fill_active_channels(channel_list* active_channels,int num_active);

	typedef std::vector<pollfd> pollfd_list;
	typedef std::map<fd_t, channel*> channel_map;
	//pollfd的数组，用来维护poll函数所需要的pollfd数组
	pollfd_list pollfd_list_;
	//用来查询特定fd_t对应的channel,从而通知channel进行相关的操作
	channel_map channel_map_;
	//所属的event_loop的指针
	event_loop* owner_loop_;
};
