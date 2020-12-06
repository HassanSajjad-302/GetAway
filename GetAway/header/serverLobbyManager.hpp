#ifndef GETAWAY_SERVERLOBBYMANAGER_HPP
#define GETAWAY_SERVERLOBBYMANAGER_HPP




#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <iostream>
#include <map>
#include <tuple>
#include "session.hpp"
#include "serverListener.hpp"
#include "serverGameManager.hpp"
#include "messageTypeEnums.hpp"
#include "playerData.hpp"
#include "deckSuit.hpp"

class serverLobbyManager
{
    typedef typeof(playerData::cards) cardType;
    std::vector<cardType *> debug;
    std::map<int, std::tuple<const std::string,
    std::shared_ptr<session<serverLobbyManager, true>>>> gameData;

    //Following are used for game management only.
    bool gameStarted = false;
    bool firstRound;
    std::map<deckSuit, std::set<int>> flushedCards;
    deckSuit suitOfTheRound;

    std::vector<playerData> gamePlayersData;
    std::vector<std::tuple<int, Card>> roundTurns; //id and Card
    //

    std::shared_ptr<serverListener> serverlistener; //This is passed next to lobby which uses it to cancel accepting
    std::shared_ptr<serverGameManager> nextManager;

    std::string playerNameAdvanced;
    std::string playerNameFinal;
public:
    void setPlayerNameAdvanced(std::string advancedPlayerName);

public:
    explicit
    serverLobbyManager(std::shared_ptr<serverListener> serverlistener_);
    void uselessWriteFunction(int id);

    //Used-By-Session
    int join  (std::shared_ptr<session<serverLobbyManager, true>> lobbySession);
    void leave (int id);
    void packetReceivedFromNetwork(std::istream &in, int receivedPacketSize, int sessionId);

    void sendPLAYERJOINEDToAllExceptOne(int excitedSessionId);

    void sendPLAYERLEFTToAllExceptOne(int excitedSessionId);

    void sendCHATMESSAGEIDToAllExceptOne(const std::string &chatMessageReceived, int excitedSessionId);

    void sendGAMETURNSERVERTOAllExceptOne(int sessionId, Card card);

    void managementJoin(int excitedSessionId);

    void managementCHATMESSAGEReceived(const std::string &chatMessageReceived, int excitedSessionId);

    void startGame();

    void doFirstTurnOfFirstRound();

    void initializeGame();

    void managementGAMETURNCLIENTReceived(int sessionId, Card cardReceived);

    void doTurnReceivedOfFirstRound(std::vector<playerData>::iterator turnReceivedPlayer, Card cardReceived);

    void newRoundTurn(std::vector<playerData>::iterator currentGamePlayer);

    void Turn(std::vector<playerData>::iterator currentTurnPlayer, Card card);

    void
    performFirstOrMiddleTurn(std::vector<playerData>::iterator currentTurnPlayer, Card card, bool firstTurn);

    void
    performLastOrThullaTurn(std::vector<playerData>::iterator currentTurnPlayer, Card card, bool lastTurn);

    void
    turnCardNumberOfGamePlayerIterator(std::vector<playerData>::iterator turnReceivedPlayer, Card card);

    bool indexGamePlayerDataFromId(int id, int &index);

    std::vector<playerData>::iterator roundKingGamePlayerDataIterator();

};
#endif //GETAWAY_SERVERLOBBYMANAGER_HPP
