

#ifdef ANDROID
#ifndef MY_APPLICATION_ANDROIDSERVERMAIN_HPP
#define MY_APPLICATION_ANDROIDSERVERMAIN_HPP

#include "asio/io_context.hpp"
#include <thread>

class androidServerMain {
    asio::io_context& io;
    std::thread thr;
public:
    androidServerMain(asio::io_context& io_);
    void run();
    ~androidServerMain();
};


#endif //MY_APPLICATION_ANDROIDSERVERMAIN_HPP
#endif