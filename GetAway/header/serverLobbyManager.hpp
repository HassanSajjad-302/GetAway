#ifndef GETAWAY_SERVERLOBBYMANAGER_HPP
#define GETAWAY_SERVERLOBBYMANAGER_HPP

#include <memory>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <iostream>
#include <map>
#include <tuple>
#include "session.hpp"


class serverLobbyManager
{
    //TODO
    //Should be supplied from somewhere else
    std::chrono::seconds timePerTurn = std::chrono::seconds(60);
    std::map<int, std::tuple<std::reference_wrapper<const std::string>,
    std::shared_ptr<session<serverLobbyManager>>>> gameData;

    friend std::ostream& operator<<(std::ostream& out, serverLobbyManager& state);
    //friend std::istream& operator>>(std::istream& in, serverLobbySessionState& state);



public:
    explicit
    serverLobbyManager();

    int join  (std::shared_ptr<session<serverLobbyManager>> lobbySession, const std::string& playerName);
    void leave (int id);
};
#endif //GETAWAY_SERVERLOBBYMANAGER_HPP
