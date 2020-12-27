
#ifndef GETAWAY_ANDROIDCLIENTMAIN_HPP
#define GETAWAY_ANDROIDCLIENTMAIN_HPP

#include "asio/io_context.hpp"
#include <thread>


class androidClientMain {
    asio::io_context& io;
    std::thread thr;
public:
    androidClientMain(asio::io_context& io_);
    void run();
    ~androidClientMain();
};


#endif //GETAWAY_ANDROIDCLIENTMAIN_HPP
