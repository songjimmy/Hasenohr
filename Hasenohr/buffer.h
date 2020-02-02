#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <assert.h>
#include <unistd.h>

#include <string.h>
#include <sys/uio.h>
#include <sys/socket.h>

#include<muduo/base/Logging.h>
class buffer
{
public:
    static const size_t prepend_size=8;
    static const size_t init_size=1024;
    buffer();
    void swap(buffer& buf);
    void clear();
    void append(const char* content, size_t len);
    void append(const std::string& content);
    void retrieve(size_t len);
    void retrieve_all();
    //
    ssize_t read_fd(const int& fd, int* erro_no);
    ssize_t write_fd(const int& fd, int* erro_no);
    ssize_t write_fd(const int& fd, ssize_t send_size, int* erro_no);
    std::string recv_as_string();
    size_t size() const;
    size_t readable_size() const;
    size_t writeable_size() const;
private:
	/*
   |<---------------------------------vector<char>-------------------------------------->|
   +-----------+---------------------------+---------------------------------------------+
   |prependable|         content           |                  free space                 |
   |		   |						   |											 |
   +-----------+---------------------------+---------------------------------------------+
   0        readable_index          writeable_index                                     size
   |<         >|<------readabel size------>|<---------------writeable size-------------->|
    */

    size_t write_able_check(size_t append_size) const;
    std::vector<char>::iterator begin();
    std::vector<char>::iterator read_begin();
    std::vector<char>::iterator write_begin();
    std::vector<char>::iterator end();
    size_t readable_index_;
    size_t writeable_index_;
    std::vector<char> buffer_; 
};