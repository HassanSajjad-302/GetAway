
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

//This class is the core of the game. Important Function is packetReceivedFromNetwork(). This function is
//called by serverLobby whenever it receives a packet of mtc::GAME defined in messageTypeEnums.hpp.
//Other important function is constructor which is called when game starts from the serverLobby.
//Important member variables are lobby and players.
class serverBluff{
    serverLobby& lobby; //This lobby will call packetReceivedFromNetwork() and will also be used when our game ends.
    const std::map<int, std::tuple<std::string,
            std::unique_ptr<serverSession<serverLobby>>>>& players; //This map consists of the id per tuple of
            //playerName and unique_ptr of serverSession<serverLobby>

    bool firstTurnOfRoundExpected;
    std::map<deckSuit, std::set<int>> flushedCards;
    deckSuit suitOfTheRound;
    std::vector<bluffPData> gamePlayersData;
    int whoWillTurnNext_GamePlayersDataIndex;
    int lastPlayerWhoTurnedId;
    bool aPlayerIsMarkedForRemoval;
    int markedPlayerForRemovalId;
    int passTurnCount;
    int numberOfCardsTurned;
    std::vector<Card> cardsOnTable;
public:
    explicit
    serverBluff(const std::map<int, std::tuple<std::string,
            std::unique_ptr<serverSession<serverLobby>>>>& gameData_, serverLobby& lobbyManager_); //This constructor
            //is used by serverLobby to pass the gamedata_ and register itself to the game it starts.

    //in is used for reading message. See the function code for example.
    void packetReceivedFromNetwork(std::istream &in, int receivedPacketSize, int sessionId);

    void doFirstTurnOfFirstRound();
#ifndef NDEBUG
    void checkForCardsCount();
#endif
    //This function distributes cards randomly and gamePlayersData vector is populated in this function.
    //In gamePlayersData, players are present in turn sequence i.e. player who will play first is first in the vector,
    //player who will play later is later in the vector.
    void initializeGame();
    bool indexGamePlayerDataFromId(int id, int &index);

    void incrementWhoWillTurnNext_GamePlayersDataIndex();
};


#endif //Bluff_SERVERBLUFF_HPP
