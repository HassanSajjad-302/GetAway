//
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at tcp://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: tcps://github.com/vinniefalco/CppCon2018
//

#ifndef CPPCON2018_TCP_SESSION_HPP
#define CPPCON2018_TCP_SESSION_HPP

#include "shared_state.hpp"
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <boost/asio.hpp>


namespace net = boost::asio;
using namespace net::ip;
using error_code = boost::system::error_code;


/** Represents an established TCP connection
*/
class tcp_session : public std::enable_shared_from_this<tcp_session>
{
    inline static int count = 0;
    tcp::socket socket_;
    std::shared_ptr<shared_state> state_;
    std::vector<std::shared_ptr<std::string const>> queue_;
    std::string msg;
    //This Works
    //boost::asio::streambuf b;
    //This is New
    std::string p = "dssd";
    void func(){
        std::string p = "dsd";
        boost::asio::dynamic_string_buffer b(p);
    }
    std::string line;


    void fail(error_code ec, char const* what);
    void on_read(error_code ec, std::size_t);
    void on_write(error_code ec, std::size_t);

public:
    int id_;
    tcp_session(
        tcp::socket socket,
        std::shared_ptr<shared_state> const& state);
    ~tcp_session();
    void run();
    void session_send(std::shared_ptr<std::string const> const& ss);

};

#endif
