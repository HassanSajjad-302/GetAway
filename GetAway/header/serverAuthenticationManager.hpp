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
class serverLobbySession;
// Represents the shared server state
class serverAuthenticationManager
{
    std::string password;
    std::string playerName;
    int classSendSize =0;
    bool passwordMatched = false;

    std::shared_ptr<serverListener> serverlistener; //This is passed next to lobby which uses it to cancel accepting
    std::shared_ptr<serverLobbyManager> nextManager{std::make_shared<serverLobbyManager>(serverlistener)};


public:
    //connections before starting game.
    explicit
    serverAuthenticationManager(std::string password, std::shared_ptr<serverListener> serverlistener_);
    void join(std::shared_ptr<session<serverLobbyManager>>);
    void authentication(tcp::socket sok);
};

#endif //GETAWAY_CLIENTLOBBYMANAGER_HPP
