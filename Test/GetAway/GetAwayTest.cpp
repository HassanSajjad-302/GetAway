//
// Created by hassan on 2/3/21.
//

#include "GetAwayTest.hpp"
#include "asio.hpp"
GetAwayTest::GetAwayTest() {
    constexpr static const char arg1[] = "/usr/bin/ls";
    constexpr static const char arg2[] = "-l";
    asio::io_context io;
    inferior<arg1, arg2> f(io);
}
