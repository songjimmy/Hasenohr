#pragma once
#include "event_loop.h"
#include "event_loop_thread.h"

#include <vector>
class event_loop_thread_pool
{
public:
	event_loop_thread_pool(size_t threads_size_, event_loop* base_loop__);
	event_loop_thread_pool(const event_loop_thread_pool&) = delete;
	void operator=(const event_loop_thread_pool&) = delete;
	void start();
	event_loop* get_next_loop();
	void set_thread_size(int threads_size__);
private:
	event_loop* base_loop_;
	int next;
	bool started;
	std::vector<event_loop*> loops_;
	std::vector<event_loop_thread*> threads;
	size_t threads_size_;
};