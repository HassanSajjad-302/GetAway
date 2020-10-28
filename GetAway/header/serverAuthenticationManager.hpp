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

//TODO
//Following constant should be supplied from somewhere else.
    const int nameLength = 60; //Maximum chars for name

    friend std::istream& operator>>(std::istream& in, serverAuthenticationManager& state);


public:
    std::shared_ptr<serverLobbyManager> nextState{std::make_shared<serverLobbyManager>()};
    std::shared_ptr<serverListener> serverlistener; //This is passed next to lobby which uses it to cancel accepting
    //connections before starting game.
    explicit
    serverAuthenticationManager(std::string password, std::shared_ptr<serverListener> serverlistener_);

    [[nodiscard]] int getClassReceiveSize() const;
    [[nodiscard]] int getClassSendSize() const;
    [[nodiscard]] int getMinimumReceivedBytes() const;
    [[nodiscard]] std::string getPlayerName() const;
    void setClassSendSize(int size);
    void authentication(tcp::socket sok);
};

#endif //GETAWAY_CLIENTLOBBYMANAGER_HPP
