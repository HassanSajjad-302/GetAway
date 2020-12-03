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
class clientLobbyManager : inputRead {
    int * debug;
    std::string playerName;
    int id = 0;
    std::map<int, std::string> gamePlayers;
    std::shared_ptr<session<clientLobbyManager>> clientLobbySession;
    std::vector<lobbyMessageType> messageTypeExpected;
    inputType inputTypeExpected;

    void input(std::string inputString, inputType inputReceivedType) override;

    //TODO
    //Following two are used for print by sati.cpp
    //Should be part of some other struct
    std::string chatMessageString;
    int chatMessageInt{};

    //Following Are Used For Game Management
    std::map<deckSuit, std::set<int>> myCards; //here cards are stored based on there 0-12 number and 0-4 enum value as in
    std::map<int, int> numberOfCards; //numberOfCards for each player
    std::vector<int> turnSequence;
    std::vector<Card> turnAbleCards;
    std::vector<int> waitingForTurn; //used for first turn
    bool firstRound = false;

    std::vector<std::tuple<int, Card>> roundTurns; //id and Card

    bool badranga = false;
    deckSuit suitOfTheRound = static_cast<deckSuit>(-1);
    int senderIdExpected; //used only if firstRound = false
    //
    std::map<deckSuit, std::set<int>> flushedCards; //it will be used in the game ending to confirm the bug free gameplay.


public:
    explicit
    clientLobbyManager();

    ~clientLobbyManager();

    //Used-By-Session
    void join(std::shared_ptr<session<clientLobbyManager>> clientLobbySession_);

    void packetReceivedFromNetwork(std::istream &in, int receivedPacketSize);

    void managementLobbyReceived();

    void managementGAMEFIRSTTURNSERVERReceived();

    void uselessWriteFunctionCHATMESSAGE();

    void exitGame();

    inline void setInputType(inputType inputType);

    bool inputHelper(const std::string &inputString, int lower, int upper, inputType notInRange_,
                     inputType invalidInput_, int &input);

    void managementGAMETURNSERVERReceived(int senderId, Card cardReceived);

    void sendCHATMESSAGE();

    void sendGAMETURNCLIENT(Card card);

    void uselessWriteFunctionGAMETURNCLIENT();

    int nextInTurnSequence(int currentSessionId);

    int roundKing();

    void helperLastTurnAndThullaTurn(int nextTurnId, bool thullaTurn);

    void helperFirstTurnAndMiddleTurn(int senderId, Card card);

    void setInputTypeGameInt();

    void assignToTurnAbleCards();

    void assignToTurnAbleCards(deckSuit suit);
};

#endif //GETAWAY_CLIENTLOBBYMANAGER_HPP
