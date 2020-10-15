//
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/CppCon2018
//

#include<iostream>
#include<memory>
#include<system_error>
#include<boost/asio.hpp>
#include "listener.hpp"
#include "tcp_session.hpp"


namespace net = boost::asio;
using namespace net::ip;
using error_code = boost::system::error_code;


listener::
listener(
    net::io_context& ioc,
    tcp::endpoint endpoint,
    std::shared_ptr<shared_state> const& state)
    : acceptor_(ioc,endpoint)
    , socket_(ioc)
    , state_(state)
{
}

void
listener::
run()
{
    // Start accepting a connection
    acceptor_.async_accept(
        socket_,
        [self = shared_from_this()](error_code ec)
        {
            self->on_accept(ec);
        });
}

// Report a failure
void
listener::
fail(error_code ec, char const* what)
{
    // Don't report on canceled operations
    if(ec == net::error::operation_aborted)
        return;
    std::cerr << what << ": " << ec.message() << "\n";
}

// Handle a connection
void
listener::
on_accept(error_code ec)
{
    if(ec)
        return fail(ec, "accept");
    else
        // Launch a new session for this connection
        std::make_shared<tcp_session>(
            std::move(socket_),
            state_)->run();

    // Accept another connection
    acceptor_.async_accept(
        socket_,
        [self = shared_from_this()](error_code ec)
        {
            self->on_accept(ec);
        });
}
