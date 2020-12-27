#ifndef GETAWAY_CLIENTLOBBYMANAGER_HPP
#define GETAWAY_SERVERTCPSESSIONSTATE_HPP

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include "serverLobbyManager.hpp"
#include "serverListener.hpp"
// Represents the shared server state
class serverAuthManager
{
    asio::io_context& io;
    std::string password;
    int classSendSize =0;
    std::map<int, std::shared_ptr<session<serverAuthManager, true>>> serverAuthSessions;
    std::shared_ptr<serverListener> serverlistener; //This is passed next to lobby which uses it to cancel accepting
    std::shared_ptr<serverLobbyManager> nextManager;

public:
    //Used-By-Session
    int join(std::shared_ptr<session<serverAuthManager, true>> authSession);
    void leave(int id);
    void packetReceivedFromNetwork(std::istream &in, int receivedPacketSize, int excitedSessionId);
    //connections before starting game.
    explicit
    serverAuthManager(std::string password, std::shared_ptr<serverListener> serverlistener_, asio::io_context& io_);
    void shutDown();
};

#endif //GETAWAY_CLIENTLOBBYMANAGER_HPP
