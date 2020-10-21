#ifndef CPPCON2018_LISTENER_HPP
#define CPPCON2018_LISTENER_HPP

#include <memory>
#include <string>

#include<boost/asio/ip/tcp.hpp>
namespace net = boost::asio;
using namespace net::ip;
using errorCode = boost::system::error_code;
// Forward declaration
class clientState;

// Accepts incoming connections and launches the sessions
class clientConnector : public std::enable_shared_from_this<clientConnector>
{
    tcp::socket socket_;
    std::shared_ptr<clientState> state;

    void fail(errorCode ec, char const* what);
    void onAccept(errorCode ec);

public:
    clientConnector(
        net::io_context& ioc,
        const tcp::endpoint& endpoint,
        std::shared_ptr<clientState>  state);

    // Start accepting incoming connections
    void run();
};

//TODO
//This file is not required. Remove it.
#endif
