#ifndef GETAWAY_SERVERLISTENER_HPP
#define GETAWAY_SERVERLISTENER_HPP

#include <memory>
#include <string>

#include"asio/ip/tcp.hpp"
#ifdef ANDROID
#include "satiAndroid.hpp"
#else
#include "sati.hpp"
#endif
using namespace asio::ip;
using errorCode = asio::error_code;
// Forward declaration
class serverAuthManager;

// Accepts incoming connections and launches the sessions
class serverListener : public std::enable_shared_from_this<serverListener>, inputRead
{
    tcp::acceptor acceptor;
    std::shared_ptr<serverAuthManager> nextManager;
    std::string password;
    void fail(errorCode ec, char const* what);
    void onAccept(errorCode ec);

    void input(std::string inputString, inputType inputReceivedType) override;

public:
    tcp::socket sock;
    serverListener(
        asio::io_context& ioc,
        const tcp::endpoint& endpoint,
        std::string password_);

    // Start accepting incoming connections
    void run();
    void runAgain();
    void registerForInputReceival();
    void shutdown();

    void shutdownAcceptor();
};

#endif //GETAWAY_SERVERLISTENER_HPP
