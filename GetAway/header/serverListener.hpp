#ifndef CPPCON2018_LISTENER_HPP
#define CPPCON2018_LISTENER_HPP

#include <memory>
#include <string>

#include<boost/asio/ip/tcp.hpp>
namespace net = boost::asio;
using namespace net::ip;
using errorCode = boost::system::error_code;
// Forward declaration
class serverTcpSessionState;

// Accepts incoming connections and launches the sessions
class serverListener : public std::enable_shared_from_this<serverListener>
{
    tcp::acceptor acceptor;
    tcp::socket sock;
    std::shared_ptr<serverTcpSessionState> state;

    void fail(errorCode ec, char const* what);
    void onAccept(errorCode ec);

public:
    serverListener(
        net::io_context& ioc,
        const tcp::endpoint& endpoint,
        std::shared_ptr<serverTcpSessionState>  state);

    // Start accepting incoming connections
    void run();
};

#endif
