#ifndef LOBBY_HPP
#define LOBBY_HPP


#include "clientLobbyManager.hpp"
#include "clientTcpSessionState.hpp"
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <boost/asio.hpp>


namespace net = boost::asio;
using namespace net::ip;
using errorCode = boost::system::error_code;

//TODO
//client lobby session and clientlobbysession state classes will be
//implemented after the serverlobbysession and serverlobbysessionstate


enum class messageType{
    STATE, CHAT_MESSAGE, UPDATE
};


class clientLobbySession : public std::enable_shared_from_this<clientLobbySession>
{
    tcp::socket sock;
    std::shared_ptr<clientLobbyManager> state;
    boost::asio::streambuf lobbySessionStreamBuff;
    std::ostream out{&lobbySessionStreamBuff};
    std::istream is{&lobbySessionStreamBuff};

    static void fail(errorCode ec, char const* what);
    void onRead(errorCode ec, std::size_t);
    static void onWrite(errorCode ec, std::size_t);

public:
    bool messageRequired = false;
    messageType messageRequiredType;
    clientLobbySession(tcp::socket socket, std::shared_ptr<clientLobbyManager> state);
    void readMore(errorCode ec, int bytesRead);
    void packetReceived(int consumeBytes);
    ~clientLobbySession();
    void run();
    void sessionSend(std::shared_ptr<std::string const> const& ss);

    void receiveMessage();

    void packetReceivedComposite(errorCode ec, int bytesRead, int consumingBytes);

    void sendMessage();
};



#endif // LOBBY
