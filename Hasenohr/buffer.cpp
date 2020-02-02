#include "buffer.h"

buffer::buffer()
:readable_index_(prepend_size),
writeable_index_(prepend_size),
buffer_(prepend_size+init_size,0)
{

}

void buffer::swap(buffer& buf)
{
	buffer_.swap(buf.buffer_);
	std::swap(readable_index_, buf.readable_index_);
	std::swap(writeable_index_, buf.writeable_index_);
}

void buffer::clear()
{
	buffer temp;
	swap(temp);
}

void buffer::append(const char* content, size_t len)
{
	size_t extra_size = write_able_check(len);
	if (extra_size)
	{
		buffer_.resize(buffer_.size() + extra_size, 0);
	}
	auto copy_end = std::copy(content, content+len, write_begin());
	writeable_index_ = copy_end - begin();
}

void buffer::append(const std::string& content)
{
	size_t extra_size=write_able_check(content.size());
	if (extra_size)
	{
		buffer_.resize(buffer_.size() + extra_size, 0);
	}
	auto copy_end=std::copy(content.begin(), content.end(), write_begin());
	writeable_index_ = copy_end - begin();
}

void buffer::retrieve(size_t len)
{
	readable_index_ += len;
	assert(readable_index_ <= writeable_index_);
}

void buffer::retrieve_all()
{
	readable_index_ = writeable_index_;
}

ssize_t buffer::read_fd(const int& fd, int* erro_no)
{
	char extra_buff[65536];
	bzero(extra_buff, 65536);
	iovec vec[2];
	vec[0].iov_base = buffer_.data() + writeable_index_;
	vec[0].iov_len = writeable_size();
	vec[1].iov_base = extra_buff;
	vec[1].iov_len = 65536;
	ssize_t n = readv(fd, vec, 2);
	if (n < 0)
	{
		*erro_no = errno;
	}
	else if (n <= ssize_t(writeable_size()))
	{
		writeable_index_ += n;
	}
	else 
	{
		LOG_INFO << "use extra_buff";
		const ssize_t writeable = writeable_size();
		writeable_index_ = size();
		append(extra_buff, n - writeable);
	}
	return n;
}

ssize_t buffer::write_fd(const int& fd, int* erro_no)
{
	ssize_t n=::write(fd, &(*read_begin()), readable_size());
	if (n < 0)
	{
		*erro_no = errno;
	}
	else 
	{
		LOG_INFO << n << " bytes in buff has been send";
		readable_index_ += n;
	}
	return n;
}

ssize_t buffer::write_fd(const int& fd, ssize_t send_size, int* erro_no)
{
	ssize_t send_size_ = send_size < ssize_t(readable_size()) ? send_size : readable_size();
	ssize_t n = ::write(fd, &(*read_begin()), send_size_);
	if (n < 0)
	{
		*erro_no = errno;
	}
	else
	{
		LOG_INFO << n << " bytes in buff has been send";
		readable_index_ += n;
	}
	return n;
}

std::string buffer::recv_as_string()
{
	std::string ret(read_begin(), write_begin());
	retrieve_all();
	return ret;
}

size_t buffer::size() const
{
	return buffer_.size();
}

size_t buffer::readable_size() const
{
	return writeable_index_- readable_index_;
}

size_t buffer::writeable_size() const
{
	return size()- writeable_index_;
}

size_t buffer::write_able_check(size_t append_size) const
{
	return append_size > writeable_size() ? append_size - writeable_size() : 0;
}

std::vector<char>::iterator buffer::begin()
{
	return buffer_.begin();
}

std::vector<char>::iterator buffer::read_begin()
{
	return begin()+readable_index_;
}

std::vector<char>::iterator buffer::write_begin()
{
	return begin()+writeable_index_;
}

std::vector<char>::iterator buffer::end()
{
	return buffer_.end();
}



