#include<iostream>
#include<memory>
#include<system_error>
#include <utility>
#include "serverListener.hpp"
#include "session.hpp"
#include "serverAuthManager.hpp"
#include "Log_Macro.hpp"
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
#ifdef LOG
    spdlog::info("{}\t{}\t{}",__FILE__,__FUNCTION__ ,__LINE__);
#endif
    nextManager = std::make_shared<serverAuthManager>(std::move(password), shared_from_this());
    // Start accepting a connection
    acceptor.async_accept(
            sock,
            [self = shared_from_this()](errorCode ec)
        {
            self->onAccept(ec);
        });
#ifdef LOG
    spdlog::info("{}\t{}\t{}",__FILE__,__FUNCTION__ ,__LINE__);
#endif
}

// Report a failure
void
serverListener::
fail(errorCode ec, char const* what)
{
#ifdef LOG
    spdlog::info("{}\t{}\t{}",__FILE__,__FUNCTION__ ,__LINE__);
#endif
    // Don't report on canceled operations
    if(ec == net::error::operation_aborted)
        return;
    std::cerr << what << ": " << ec.message() << "\n";
#ifdef LOG
    spdlog::info("{}\t{}\t{}",__FILE__,__FUNCTION__ ,__LINE__);
#endif
}

// Handle a connection
void
serverListener::
onAccept(errorCode ec)
{
#ifdef LOG
    spdlog::info("{}\t{}\t{}",__FILE__,__FUNCTION__ ,__LINE__);
#endif
    if(ec)
        return fail(ec, "accept");
    else
        // Launch a new session for this connection
        std::make_shared<session<serverAuthManager,true>>(
                std::move(sock),
                nextManager)->registerSessionToManager();

    // Accept another connection
    acceptor.async_accept(
            sock,
            [self = shared_from_this()](errorCode ec)
        {
            self->onAccept(ec);
        });
#ifdef LOG
    spdlog::info("{}\t{}\t{}",__FILE__,__FUNCTION__ ,__LINE__);
#endif
}
