//
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/CppCon2018
//

#include "tcp_session.hpp"
#include <iostream>
#include <boost/asio.hpp>
#include <regex>
#include <boost/regex.h>

namespace net = boost::asio;
using namespace net::ip;
using error_code = boost::system::error_code;
//------------------------------------------------------------------------------

tcp_session::
tcp_session(
    tcp::socket socket,
    std::shared_ptr<shared_state> const& state)
    : socket_(std::move(socket))
    , state_(state)
{
    state_->join(*this);
    id_=++tcp_session::count;
    assert(count >= state->number_of_sessions());
    std::cout<<count<<std::endl;
}
tcp_session::~tcp_session(){
    state_->leave(*this);
}

void
tcp_session::
run()
{
    // Read a request
    net::async_read_until(socket_, b, '\n',
        [self = shared_from_this()]
            (error_code ec, std::size_t bytes)
        {
            self->on_read(ec, bytes);
        });
}

// Report a failure
void
tcp_session::
fail(error_code ec, char const* what)
{
    // Don't report on canceled operations
    if(ec == net::error::operation_aborted)
        return;

    std::cerr << what << ": " << ec.message() << "\n";
}

void
tcp_session::
on_read(error_code ec, std::size_t t)
{
    // Handle the error, if any
    if(ec)
        return fail(ec, "read");
    if (!ec)
    {
      std::istream is(&b);
      std::getline(is, msg);
      state_->state_send(msg+"\n",this);
      b.consume(t);
    }
    // Read another request
    net::async_read_until(socket_, b,'\n',
        [self = shared_from_this()]
            (error_code ec, std::size_t bytes)
        {
            self->on_read(ec, bytes);
        });
}

void
tcp_session::
session_send(std::shared_ptr<std::string const> const& ss)
{
    net::async_write(
        socket_,
        //net::buffer(*queue_.front()),
         net::buffer(*ss),
        [sp = shared_from_this()](
            error_code ec, std::size_t bytes)
        {
            sp->on_write(ec, bytes);
        });
}

void
tcp_session::
on_write(error_code ec, std::size_t)
{
    // Handle the error, if any
    if(ec)
        return fail(ec, "write");

}
