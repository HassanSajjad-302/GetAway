//
// Created by hassan on 12/21/2020.
//

#ifndef MY_APPLICATION_ANDROIDSERVERMAIN_HPP
#define MY_APPLICATION_ANDROIDSERVERMAIN_HPP

#include "asio/io_context.hpp"
#include <functional>
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
