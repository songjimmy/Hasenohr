#include "event_loop_thread_pool.h"
event_loop_thread_pool::event_loop_thread_pool(size_t threads_size__, event_loop* base_loop__)
	:base_loop_(base_loop__),next(0),started(false),threads_size_(threads_size__)
{
}
void event_loop_thread_pool::start()
{
	assert(!started);
	base_loop_->assert_in_loop_thread();
	started = true;
	for (size_t i = 0; i < threads_size_; i++)
	{
		threads.push_back(new event_loop_thread);
		loops_.push_back(threads.back()->loop());
	}
}
event_loop* event_loop_thread_pool::get_next_loop()
{
	base_loop_->assert_in_loop_thread();
	next = next % int(threads_size_);
	return loops_[next++];
}

void event_loop_thread_pool::set_thread_size(int threads_size__)
{
	base_loop_->assert_in_loop_thread();
	assert(!started);
	threads_size_ = threads_size__;
}
