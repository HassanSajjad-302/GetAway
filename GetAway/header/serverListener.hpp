#ifndef GETAWAY_SERVERLISTENER_HPP
#define GETAWAY_SERVERLISTENER_HPP

#include <memory>
#include <string>

#include"asio/ip/tcp.hpp"
#include "asio/ip/udp.hpp"
#include "terminalInputBase.hpp"

using namespace asio::ip;
using errorCode = asio::error_code;
// Forward declaration
class serverAuthManager;

// Accepts incoming connections and launches the sessions
class serverListener : public std::enable_shared_from_this<serverListener>, terminalInputBase
{
    class PF {
    public:
        static void setLobbyMainOnePlayer();
    };
    asio::io_context& io;
    tcp::acceptor acceptor;
    std::shared_ptr<serverAuthManager> nextManager;
    std::string password;
    std::string serverName;

    //Following are used for local server find handling
    udp::socket probeListenerUdpSock;
    udp::endpoint probeListenerEndpoint;
    udp::endpoint remoteEndpoint{};
    char recieveBuffer[512];

    static void fail(errorCode ec, char const* what);
    void onAccept(errorCode ec);

    void input(std::string inputString, inputType inputReceivedType) override;

public:
    tcp::socket tcpSock;
    serverListener(
            asio::io_context& io_,
            const tcp::endpoint& endpoint,
            const std::string& serverName_,
            std::string password_);

    // Start accepting incoming connections
    void run();
    void runAgain();
    void probeReply(asio::error_code ec, int size);

    void registerForInputReceival();
    void shutdown();

    void shutdownAcceptorAndProbe();
};

#endif //GETAWAY_SERVERLISTENER_HPP
