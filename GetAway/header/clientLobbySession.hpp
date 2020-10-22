#ifndef LOBBY_HPP
#define LOBBY_HPP


#include "clientLobbySessionState.hpp"
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

/** Represents an established TCP connection
*/
class clientLobbySession : public std::enable_shared_from_this<clientLobbySession>
{
    tcp::socket sock;
    std::shared_ptr<clientLobbySessionState> state;
    boost::asio::streambuf lobbySessionStreamBuff;

    static void fail(errorCode ec, char const* what);
    void onRead(errorCode ec, std::size_t);
    static void onWrite(errorCode ec, std::size_t);

public:
    clientLobbySession(tcp::socket socket, std::shared_ptr<clientLobbySessionState> state);
    ~clientLobbySession();
    void run();
    void sessionSend(std::shared_ptr<std::string const> const& ss);

};



#endif // LOBBY
