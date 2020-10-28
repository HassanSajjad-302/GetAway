#ifndef GETAWAY_CLIENTLOBBYMANAGER_HPP
#define GETAWAY_CLIENTLOBBYSESSIONSTATE_HPP


#include <memory>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <iostream>
#include <map>
#include <tuple>
#include "sessionID.hpp"


class serverLobbyManager
{
    //TODO
    //This should be supplied by serverTcpSessionState. But currently
    //not doing this, as huge overall design change is expected.
    std::chrono::seconds timePerTurn = std::chrono::seconds(60);
    std::map<int, std::tuple<std::reference_wrapper<const std::string>,
    std::reference_wrapper<sessionID<serverLobbyManager>>>> gameData;

    friend std::ostream& operator<<(std::ostream& out, serverLobbyManager& state);
    //friend std::istream& operator>>(std::istream& in, serverLobbySessionState& state);


public:
    void customer(tcp::socket sok);
    explicit
    serverLobbyManager();

    int getClassWriteSize();
    int join  (sessionID<serverLobbyManager>& session, const std::string& playerName);
    void leave (int id);

    void broadcastState();
};
#endif //GETAWAY_CLIENTLOBBYMANAGER_HPP
