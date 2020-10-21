#ifndef LOBBY_HPP
#define LOBBY_HPP



#include "clientState.hpp"
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
class clientLobbySession : public std::enable_shared_from_this<clientLobbySession>
{
    tcp::socket socket_;
    std::shared_ptr<clientState> state;
    boost::asio::streambuf lobbySessionStreamBuff;
    std::string player;
    int id =0;
    friend void clientState::join(clientLobbySession& session, const std::string& playerName);
    friend void clientState::leave(clientLobbySession& session);
    static void fail(errorCode ec, char const* what);
    void onRead(errorCode ec, std::size_t);
    static void onWrite(errorCode ec, std::size_t);

public:
    clientLobbySession(const std::string& playerName, tcp::socket socket, std::shared_ptr<clientState> state);
    ~clientLobbySession();
    void run();
    void sessionSend(std::shared_ptr<std::string const> const& ss);

};



#endif // LOBBY
