#ifndef GETAWAY_SERVERLISTENER_HPP
#define GETAWAY_SERVERLISTENER_HPP

#include <memory>
#include <string>

#include"asio/ip/tcp.hpp"
#include "asio/ip/udp.hpp"
#include "terminalInputBase.hpp"
#include "serverLobby.hpp"
#include "clientLobby.hpp"
#include "constants.hpp"
using namespace asio::ip;
using errorCode = asio::error_code;

// Accepts incoming connections and launches the sessions
class serverListener : public std::enable_shared_from_this<serverListener>, terminalInputBase
{
    class PF {
    public:
        static void setLobbyMainOnePlayer();
    };
    asio::io_context& io;
    tcp::acceptor acceptor;
    constants::gamesEnum gameSelected;
public:
    serverLobby nextManager;
private:
    std::string serverName;

    tcp::endpoint acceptorEndpoint;
    //Following are used for local server find handling
    udp::socket probeListenerUdpSock;
    udp::endpoint probeListenerEndpoint;
    udp::endpoint remoteEndpoint{};
    char recieveBuffer[512];

    static void fail(errorCode ec, char const* what);
    void onAccept(errorCode ec);

    void input(std::string inputString, inputType inputReceivedType) override;
    std::shared_ptr<serverListener> ptr;

    tcp::socket tcpSockAcceptor;
    tcp::socket tcpSockClient;
    bool serverOnly = true;
    std::string clientName = "Player";
    clientSession<clientLobby, asio::io_context&, std::string, serverListener*, bool, constants::gamesEnum>* clientPtr;
public:
    serverListener(asio::io_context& io_, const tcp::endpoint& acceptorEndpoint_, std::string  serverName_,
                   constants::gamesEnum gameSelected_);

    serverListener(
            asio::io_context& io_,
            const tcp::endpoint& acceptorEndpoint_,
            std::string  serverName_,
            std::string clientName,
            constants::gamesEnum gameSelected_);
    // Start accepting incoming connections
    void run();
    void runAgain();
    void probeReply(asio::error_code ec, int size);

    void registerForInputReceival();
    void closeAcceptorAndShutdown();
    void shutdown();

    void shutdownAcceptorAndProbe();

    void promote();

    void tcpSockClientConnectToServerFail(errorCode ec);

    void startTheGame();
};

#endif //GETAWAY_SERVERLISTENER_HPP
