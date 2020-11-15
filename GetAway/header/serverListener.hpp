#ifndef GETAWAY_SERVERLISTENER_HPP
#define GETAWAY_SERVERLISTENER_HPP

#include <memory>
#include <string>

#include<boost/asio/ip/tcp.hpp>
namespace net = boost::asio;
using namespace net::ip;
using errorCode = boost::system::error_code;
// Forward declaration
class serverAuthManager;

// Accepts incoming connections and launches the sessions
class serverListener : public std::enable_shared_from_this<serverListener>
{
    tcp::acceptor acceptor;
    std::shared_ptr<serverAuthManager> nextManager;
    std::string password;
    void fail(errorCode ec, char const* what);
    void onAccept(errorCode ec);

public:
    tcp::socket sock;
    serverListener(
        net::io_context& ioc,
        const tcp::endpoint& endpoint,
        std::string password_);

    // Start accepting incoming connections
    void run();
};

#endif //GETAWAY_SERVERLISTENER_HPP
