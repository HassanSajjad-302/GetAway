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
#include "playerCards.hpp"
struct deckSuitValue{
public:
    static const int CLUB = 0;
    static const int HEART = 1;
    static const int SPADE = 2;
    static const int DIAMOND = 3;
    static inline std::string messageTypeStrings[4] = {"CLUB", "HEART", "SPADE", "DIAMOND"};
};

enum class deckSuit{
    CLUB = deckSuitValue::CLUB,
    HEART = deckSuitValue::HEART,
    SPADE = deckSuitValue::SPADE,
    DIAMOND = deckSuitValue::DIAMOND,
    ENUMSIZE = 5
};
enum class turnType{
    TURNBYPLAYER,
    TURNNOOTHERPOSSIBLE,
    TURNPLAYEROFFLINE
};

class turnMeta{
    std::vector<int> turnOrder; //First int is id, second is turnType;a
    int currentPlayer = 0;
public:
    void setTurnOrder(std::vector<int> Ids);
    int nextPlayerId();
    int getCurrentPlayerId();

    void setCurrentPlayer(int playerId);
};
class serverLobbyManager
{
    std::map<int, std::tuple<const std::string,
    std::shared_ptr<session<serverLobbyManager, true>>>> gameData;

    //Following 2 are used for starting the game
    net::steady_timer timer{std::get<1>(gameData.find(excitedSessionId)->second)->sock.get_executor(), std::chrono::seconds(2)};
    int numOfPlayers = 0;

    //Following are used for game management only.
    std::map<int, playerCards> playerGameCards;
    std::list<int> flushedCards;

    std::map<int, std::vector<lobbyMessageType>> messagesExpectedFromPlayers;
    //Tuple first int is id second is the value of the card. Used only for first turn.
    std::list<std::tuple<int,int>> turnAlreadyDeterminedIdsInFirstTurn;
    deckSuit suitOfTheRound;
    turnMeta turnIfo;



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
};
#endif //GETAWAY_SERVERLOBBYMANAGER_HPP
