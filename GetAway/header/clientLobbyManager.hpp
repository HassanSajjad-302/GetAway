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
    inputType inputTypeExpected;

    friend std::istream& operator>>(std::istream& in, clientLobbyManager& state);
    friend std::ostream& operator<<(std::ostream& out, clientLobbyManager& state);


    void input(std::string inputString, inputType inputReceivedType) override;

    std::string chatMessageString;
    int chatMessageInt;


    //Following Are Used For Game Management
    std::list<int> myCards;
    //Used For First Turn Only
    std::vector<int> turnAlreadyDetermined;
    bool gameStarted = false;
    bool notRunPosted = false;

    //
#ifndef NDEBUG
    int numOfRoundPlayers = 0;
    std::vector<int> flushedCards;
#endif

public:
    explicit
    clientLobbyManager();
    ~clientLobbyManager();
    //Used-By-Session
    void join(std::shared_ptr<session<clientLobbyManager>> clientLobbySession_);
    int receivedPacketSize = 0;

    void managementLobbyReceived();

    void managementGAMEFIRSTTURNSERVERReceived();

    void managementNextAction();

    void uselessWriteFunction();

    void exitGame();

    inline void setInputType(inputType inputType);
};

#endif //GETAWAY_CLIENTLOBBYMANAGER_HPP
