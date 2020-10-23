#ifndef GETAWAY_SERVERLOBBYSESSION_HPP
#define GETAWAY_SERVERLOBBYSESSION_HPP



#include "serverLobbySessionState.hpp"
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <boost/asio.hpp>


namespace net = boost::asio;
using namespace net::ip;
using errorCode = boost::system::error_code;


/** Represents an established TCP connection
*/
class serverLobbySession : public std::enable_shared_from_this<serverLobbySession>
{
    tcp::socket sock;
    std::shared_ptr<serverLobbySessionState> state;
    net::streambuf lobbySessionStreamBuff;
    std::ostream out{&lobbySessionStreamBuff};
    std::istream is{&lobbySessionStreamBuff};
    static void fail(errorCode ec, char const* what);
    void onRead(errorCode ec, std::size_t);
    static void onWrite(errorCode ec, std::size_t);

public:
    serverLobbySession(const std::string& playerName, tcp::socket socket, std::shared_ptr<serverTcpSessionState> state);
    ~serverLobbySession();
    void run();
    void sessionSend(std::shared_ptr<std::string const> const& ss);

};



#endif // GETAWAY_SERVERLOBBYSESSION_HPP
