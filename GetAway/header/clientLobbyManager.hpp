#ifndef GETAWAY_CLIENTLOBBYMANAGER_HPP
#define GETAWAY_CLIENTLOBBYMANAGER_HPP

#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <iostream>
class clientLobbySession;


// Represents the shared server state
class clientLobbyManager
{
    std::string playerName;
    int id = 0;
    std::map<int, std::string> gamePlayers;
    int classSendSize =0;

    friend std::istream& operator>>(std::istream& in, clientLobbyManager& state);

public:
    explicit
    clientLobbyManager();
    [[nodiscard]] int getClassSendSize() const;
};

#endif //GETAWAY_CLIENTLOBBYMANAGER_HPP
