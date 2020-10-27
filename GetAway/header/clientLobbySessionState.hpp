#ifndef GETAWAY_CLIENTLOBBYSESSIONSTATE_HPP
#define GETAWAY_CLIENTLOBBYSESSIONSTATE_HPP

#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <iostream>
class clientLobbySession;

// Represents the shared server state
class clientLobbySessionState
{
    std::string playerName;
    int id = 0;
    std::map<int, std::string> gamePlayers;
    int classSendSize =0;

    friend std::istream& operator>>(std::istream& in, clientLobbySessionState& state);


public:
    explicit
    clientLobbySessionState();
    [[nodiscard]] int getClassSendSize() const;
};

#endif //GETAWAY_CLIENTLOBBYSESSIONSTATE_HPP
