
#ifndef GETAWAY_ANDROIDMAIN_HPP
#define GETAWAY_ANDROIDMAIN_HPP

#include "asio/io_context.hpp"
#include <thread>



class androidMain {
    asio::io_context& io;
    std::thread thr;
public:
    androidMain(asio::io_context& io_);
    void run();
    ~androidMain();
};


#endif //GETAWAY_ANDROIDMAIN_HPP
