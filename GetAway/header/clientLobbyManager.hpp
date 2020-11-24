#ifndef GETAWAY_CLIENTLOBBYMANAGER_HPP
#define GETAWAY_CLIENTLOBBYMANAGER_HPP

#include <map>
#include <set>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <iostream>
#include "session.hpp"
#include "messageTypeEnums.hpp"
#include "sati.hpp"
#include "deckSuit.hpp"

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
    int chatMessageInt{};


    //Following Are Used For Game Management
    std::map<int, std::set<int>> myCards; //here cards are stored based on there 0-12 number and 0-4 enum value as in
    std::vector<int> turnSequence;

    std::vector<int> waitingForTurn;
    bool gameStarted = false; //not used yet
    bool badranga = false;
    deckSuit suitOfTheRound;
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

    void managementGAMEFIRSTTURNSERVERReceived(std::vector<std::tuple<int, int>> turnAlreadyDetermined_);

    void uselessWriteFunction();

    void exitGame();

    inline void setInputType(inputType inputType);

    bool inputHelper(const std::string& inputString, int lower, int upper, inputType notInRange_,
                     inputType invalidInput_, int& input);
};

#endif //GETAWAY_CLIENTLOBBYMANAGER_HPP
