#include<iostream>
#include<memory>
#include<system_error>
#include<boost/asio.hpp>
#include <utility>
#include "clientConnector.hpp"
#include "clientTcpSession.hpp"


namespace net = boost::asio;
using namespace net::ip;
using errorCode = boost::system::error_code;


clientConnector::
clientConnector(
    net::io_context& ioc,
    const tcp::endpoint& endpoint,
    std::shared_ptr<clientState>  state)
    : socket_(ioc)
    , state(std::move(state))
{
}

void
clientConnector::
run()
{
    // Start accepting a connection
    acceptor.async_accept(
        socket_,
        [self = shared_from_this()](errorCode ec)
        {
            self->onAccept(ec);
        });

    // Connects to the EndPoint
    socket_.connect(endpoint);
}

// Report a failure
void
clientConnector::
fail(errorCode ec, char const* what)
{
    // Don't report on canceled operations
    if(ec == net::error::operation_aborted)
        return;
    std::cerr << what << ": " << ec.message() << "\n";
}

// Handle a connection
void
clientConnector::
onAccept(errorCode ec)
{
    if(ec)
        return fail(ec, "accept");
    else
        // Launch a new session for this connection
        std::make_shared<clientTcpSession>(
                std::move(socket_),
                state)->run();

    // Accept another connection
    acceptor.async_accept(
        socket_,
        [self = shared_from_this()](errorCode ec)
        {
            self->onAccept(ec);
        });
}
