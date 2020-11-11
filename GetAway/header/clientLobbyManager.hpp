#ifndef GETAWAY_CLIENTLOBBYMANAGER_HPP
#define GETAWAY_CLIENTLOBBYMANAGER_HPP

#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <iostream>
#include "session.hpp"
#include "messageTypeEnums.hpp"
#include "sati.hpp"

// Represents the shared server state
class clientLobbyManager : inputRead
{
    std::string playerName;
    int id = 0;
    std::map<int, std::string> gamePlayers;
    std::shared_ptr<session<clientLobbyManager>> clientLobbySession;
    std::vector<lobbyMessageType> messageTypeExpected;
    friend std::istream& operator>>(std::istream& in, clientLobbyManager& state);
    friend std::ostream& operator<<(std::ostream& out, clientLobbyManager& state);

    void inputInt(int input) override;
    void inputString(std::string str) override;

    std::string chatMessageString;
    int chatMessageInt;
public:
    explicit
    clientLobbyManager();

    //Used-By-Session
    void join(std::shared_ptr<session<clientLobbyManager>> clientLobbySession_);
    int receivedPacketSize = 0;

    void managementLobbyReceived();

    void managementNextAction();

    void uselessWriteFunction();
};

#endif //GETAWAY_CLIENTLOBBYMANAGER_HPP
