#ifndef GETAWAY_CLIENTLOBBYMANAGER_HPP
#define GETAWAY_CLIENTLOBBYMANAGER_HPP

#include <map>
#include <set>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include "session.hpp"
#include "messageTypeEnums.hpp"
#include "terminalInputBase.hpp"
#include "inputType.h"
#include "deckSuit.hpp"
#include "asio/io_context.hpp"
#include "clientRoomManager.hpp"

enum whoTurned{
    CLIENT,
    RECEIVED,
    AUTO
};
// Represents the shared server state
class clientLobbyManager : terminalInputBase {
    class printingFunctions;
    clientRoomManager& roomManager;
    //asio::io_context& io;
    const std::string& playerName;
    const std::map<int, std::string>& players;
    int id = 0;
    //std::shared_ptr<session<clientLobbyManager>> clientLobbySession;
    inputType inputTypeExpected;

    void input(std::string inputString, inputType inputReceivedType) override;

    //TODO
    //Following two are used for clearAndPrint by sati.cpp
    //Should be part of some other struct
    std::string chatMessageString;
    int chatMessageInt{};

    //Following Are Used For Game Management
    std::map<deckSuit, std::set<int>> myCards; //here cards are stored based on there 0-12 number and 0-4 enum value as in
    std::map<int, int> numberOfCards; //numberOfCards for each player
    std::vector<int> turnSequence;
    std::vector<Card> turnAbleCards;
    std::vector<int> waitingForTurn; //used for first round
    bool firstRound = false;

    std::vector<std::tuple<int, Card>> roundTurns; //id and Card

    deckSuit suitOfTheRound = static_cast<deckSuit>(-1);
    int turnPlayerIdExpected; //used only if firstRound = false
    //
    std::map<deckSuit, std::set<int>> flushedCards; //it will be used in the game ending to confirm the bug free gameplay.
    bool gameStarted = false;

public:
    explicit
    clientLobbyManager(clientRoomManager &roomManager, const std::string& playerName_,
                       const std::map<int, std::string>& players_, std::istream& in);

    ~clientLobbyManager();

    void packetReceivedFromNetwork(std::istream &in, int receivedPacketSize);

    inline void setInputType(inputType inputType);

    inline void setBaseAndInputType(terminalInputBase* base_, inputType type);

    void Turn(int playerId, Card card, whoTurned who);


    void sendGAMETURNCLIENT(Card card);

    int nextInTurnSequence(int currentSessionId);

    int roundKing();

    void helperLastTurnAndThullaTurn(int nextTurnId, Card card, bool thullaTurn, whoTurned who);

    void helperFirstTurnAndMiddleTurn(int playerId, Card card, bool firstTurn, whoTurned who);

    void setInputTypeGameInt();

    void assignToTurnAbleCards();

    void assignToTurnAbleCards(deckSuit suit);

    void firstRoundTurnHelper(int playerId, Card card, whoTurned who);

    void gameExitFinished();

    void setBaseAndInputTypeFromclientChatMessage();
};

#endif //GETAWAY_CLIENTLOBBYMANAGER_HPP
