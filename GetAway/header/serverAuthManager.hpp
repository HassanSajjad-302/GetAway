#ifndef GETAWAY_CLIENTLOBBYMANAGER_HPP
#define GETAWAY_SERVERTCPSESSIONSTATE_HPP

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <iostream>
#include "serverLobbyManager.hpp"
#include "serverListener.hpp"
// Represents the shared server state
class serverAuthManager
{
    std::string password;
    int classSendSize =0;
    std::map<int, std::shared_ptr<session<serverAuthManager, true>>> serverAuthSessions;
    std::shared_ptr<serverListener> serverlistener; //This is passed next to lobby which uses it to cancel accepting
    std::shared_ptr<serverLobbyManager> nextManager;

    friend std::istream &operator>>(std::istream &in, serverAuthManager &state);
public:
    int excitedSessionId = 0;
    int receivedPacketSize = 0;

    //connections before starting game.
    explicit
    serverAuthManager(std::string password, std::shared_ptr<serverListener> serverlistener_);
    int join(std::shared_ptr<session<serverAuthManager, true>> authSession);
};

#endif //GETAWAY_CLIENTLOBBYMANAGER_HPP
