#ifndef LOBBY_HPP
#define LOBBY_HPP



#include "serverState.hpp"
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <boost/asio.hpp>


namespace net = boost::asio;
using namespace net::ip;
using errorCode = boost::system::error_code;


/** Represents an established TCP connection
*/
class serverLobbySession : public std::enable_shared_from_this<serverLobbySession>
{
    tcp::socket socket_;
    std::shared_ptr<serverState> state;
    boost::asio::streambuf lobbySessionStreamBuff;
    std::string player;
    int id =0;
    friend void serverState::join(serverLobbySession& session, const std::string& playerName);
    friend void serverState::leave(serverLobbySession& session);
    static void fail(errorCode ec, char const* what);
    void onRead(errorCode ec, std::size_t);
    static void onWrite(errorCode ec, std::size_t);

public:
    serverLobbySession(const std::string& playerName, tcp::socket socket, std::shared_ptr<serverState> state);
    ~serverLobbySession();
    void run();
    void sessionSend(std::shared_ptr<std::string const> const& ss);

};



#endif // LOBBY
