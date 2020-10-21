#include<iostream>
#include<memory>
#include<system_error>
#include<boost/asio.hpp>
#include <utility>
#include "serverListener.hpp"
#include "serverTcpSession.hpp"


namespace net = boost::asio;
using namespace net::ip;
using errorCode = boost::system::error_code;


serverListener::
serverListener(
    net::io_context& ioc,
    const tcp::endpoint& endpoint,
    std::shared_ptr<serverState>  state)
    : acceptor(ioc, endpoint)
    , socket_(ioc)
    , state(std::move(state))
{
}

void
serverListener::
run()
{
    // Start accepting a connection
    acceptor.async_accept(
        socket_,
        [self = shared_from_this()](errorCode ec)
        {
            self->onAccept(ec);
        });
}

// Report a failure
void
serverListener::
fail(errorCode ec, char const* what)
{
    // Don't report on canceled operations
    if(ec == net::error::operation_aborted)
        return;
    std::cerr << what << ": " << ec.message() << "\n";
}

// Handle a connection
void
serverListener::
onAccept(errorCode ec)
{
    if(ec)
        return fail(ec, "accept");
    else
        // Launch a new session for this connection
        std::make_shared<serverTcpSession>(
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
