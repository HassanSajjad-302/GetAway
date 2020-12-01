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
    std::map<int, std::tuple<const std::string,
    std::shared_ptr<session<serverLobbyManager, true>>>> gameData;

    //Following are used for game management only.
    bool gameStarted = false;
    bool firstRound;
    std::list<int> flushedCards;
    deckSuit suitOfTheRound;

    std::vector<playerData> gamePlayersData;
    std::vector<std::tuple<int, int>> roundTurns; //cardNumber and id
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

    void sendGAMETURNSERVERTOAllExceptOne(int receivedCardNumber, int excitedSessionId);

    void managementJoin(int excitedSessionId);

    void managementCHATMESSAGEReceived(const std::string &chatMessageReceived, int excitedSessionId);

    void startGame();

    void doFirstTurnOfFirstRound();

    void initializeGame();

    void managementGAMETURNCLIENTReceived(int receivedCardNumber, int sessionId);

    void doTurnReceivedOfFirstRound(std::vector<playerData>::iterator turnReceivedPlayer, int receivedCardNumber);

    void newRoundTurn(std::vector<playerData>::iterator currentGamePlayer);

    void Turn(std::vector<playerData>::iterator currentTurnPlayer, int receivedCardNumber);

    void
    performFirstOrMiddleTurn(std::vector<playerData>::iterator currentTurnPlayer, int receivedCardNumber,
                             bool firstTurn);

    void
    performLastOrThullaTurn(std::vector<playerData>::iterator currentTurnPlayer, int receivedCardNumber, bool lastTurn);

    void
    turnCardNumberOfGamePlayerIterator(std::vector<playerData>::iterator turnReceivedPlayer, int receivedCardNumber);

    bool indexGamePlayerDataFromId(int id, int &index);

    std::vector<playerData>::iterator roundKingGamePlayerDataIterator();

};
#endif //GETAWAY_SERVERLOBBYMANAGER_HPP
