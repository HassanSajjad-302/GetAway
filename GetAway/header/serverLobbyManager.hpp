#ifndef GETAWAY_SERVERLOBBYMANAGER_HPP
#define GETAWAY_SERVERLOBBYMANAGER_HPP




#include <memory>
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

    //Following 2 are used for starting the game
    net::steady_timer gameTimer{std::get<1>(gameData.find(excitedSessionId)->second)->sock.get_executor(), std::chrono::seconds(2)};
    int numOfPlayers = 0;

    //Following are used for game management only.
    bool gameStarted = false;
    bool firstTurn = true;
    std::list<int> flushedCards;
    deckSuit suitOfTheRound;

    std::vector<playerData> gamePlayersData;
    int currentIndexGamePlayersData;

    //used in receive handlers
    int receivedCardNumber;
    //
    friend std::ostream& operator<<(std::ostream& out, serverLobbyManager& state);
    friend std::istream& operator>>(std::istream& in, serverLobbyManager& state);

    std::shared_ptr<serverListener> serverlistener; //This is passed next to lobby which uses it to cancel accepting
    std::shared_ptr<serverGameManager> nextManager;

    std::string chatMessageReceived;
    std::string playerNameAdvanced;
    std::string playerNameFinal;
    lobbyMessageType messageSendingType;
public:
    void setPlayerNameAdvanced(std::string advancedPlayerName_);

public:
    explicit
    serverLobbyManager(std::shared_ptr<serverListener> serverlistener_);
    void uselessWriteFunction(int id);

    //Used-By-Session
    int join  (std::shared_ptr<session<serverLobbyManager, true>> lobbySession);
    int excitedSessionId;
    int receivedPacketSize;
    void leave (int id);



    void sendSelfAndStateToOneAndPlayerJoinedToRemaining();

    void sendChatMessageToAllExceptSenderItself();

    void printPlayerJoined();

    void printPlayerLeft();

    void sendPlayerLeftToAllExceptOne();

    void startGame();

    void doFirstTurn();

    void checkForAutoFirstTurn();

    void checkForNextTurn(int nextPlayerId);

    void initializeGame();

    void handlerPlayerLeft(int id);
};
#endif //GETAWAY_SERVERLOBBYMANAGER_HPP
