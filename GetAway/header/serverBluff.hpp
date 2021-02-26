
#ifndef Bluff_SERVERBLUFF_HPP
#define Bluff_SERVERBLUFF_HPP
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <map>
#include <tuple>
#include "serverSession.hpp"
#include "messageTypeEnums.hpp"
#include "bluffPData.hpp"
#include "deckSuit.hpp"
#include "asio/io_context.hpp"
class serverLobby;

class serverBluff{
    serverLobby& lobbyManager;
    const std::map<int, std::tuple<std::string,
            std::unique_ptr<serverSession<serverLobby>>>>& players;

    //bool firstRound;
    bool firstTurnOfRoundExpected;
    std::map<deckSuit, std::set<int>> flushedCards;
    deckSuit suitOfTheRound;
    std::vector<bluffPData> gamePlayersData;
    int whoWillTurnNext_GamePlayersDataIndex;
    int lastPlayerWhoTurnedId;
    bool aPlayerIsMarkedForRemoval;
    int markedPlayerForRemovalId;
    int passTurnCount;
    //std::vector<std::tuple<int, Card>> roundTurns; //id and Card
    int numberOfCardsTurned;
    std::vector<Card> cardsOnTable;
public:
    explicit
    serverBluff(const std::map<int, std::tuple<std::string,
            std::unique_ptr<serverSession<serverLobby>>>>& gameData_, serverLobby& lobbyManager_);

    void packetReceivedFromNetwork(std::istream &in, int receivedPacketSize, int sessionId);

    void doFirstTurnOfFirstRound();
#ifndef NDEBUG
    void checkForCardsCount();
#endif
    void initializeGame();
    bool indexGamePlayerDataFromId(int id, int &index);

    void incrementWhoWillTurnNext_GamePlayersDataIndex();
};


#endif //Bluff_SERVERBLUFF_HPP
