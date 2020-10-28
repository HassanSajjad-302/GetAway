#include<iostream>
#include<memory>
#include<system_error>
#include <utility>
#include "serverListener.hpp"
#include "session.hpp"
#include "serverAuthenticationManager.hpp"
namespace net = boost::asio;
using namespace net::ip;
using errorCode = boost::system::error_code;


serverListener::
serverListener(
    net::io_context& ioc,
    const tcp::endpoint& endpoint,
    std::string password_)
    : acceptor(ioc, endpoint)
    , sock(ioc)
    , password(std::move(password_))
{
}

void
serverListener::
run()
{

    nextState = std::make_shared<serverAuthenticationManager>(std::move(password), shared_from_this());
    // Start accepting a connection
    acceptor.async_accept(
            sock,
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
        std::make_shared<session<serverAuthenticationManager>>(
                std::move(sock),
                nextState)->receiveMessage(&serverAuthenticationManager::authentication);

    // Accept another connection
    acceptor.async_accept(
            sock,
            [self = shared_from_this()](errorCode ec)
        {
            self->onAccept(ec);
        });
}
