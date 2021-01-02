#ifndef GETAWAY_SERVERLOBBYMANAGER_HPP
#define GETAWAY_SERVERLOBBYMANAGER_HPP

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <map>
#include <tuple>
#include "session.hpp"
#include "serverListener.hpp"
#include "messageTypeEnums.hpp"
#include "playerData.hpp"
#include "deckSuit.hpp"
#include "asio/io_context.hpp"
class serverRoomManager;
class serverLobbyManager
{
    serverRoomManager& roomManager;
    const std::map<int, std::tuple<const std::string,
    std::shared_ptr<session<serverRoomManager, true>>>>& players;

    bool firstRound;
    std::map<deckSuit, std::set<int>> flushedCards;
    deckSuit suitOfTheRound;
    std::vector<playerData> gamePlayersData;
    std::vector<std::tuple<int, Card>> roundTurns; //id and Card

public:
    explicit
    serverLobbyManager(    const std::map<int, std::tuple<const std::string,
            std::shared_ptr<session<serverRoomManager, true>>>>& gameData_, serverRoomManager& roomManager_);

    void packetReceivedFromNetwork(std::istream &in, int receivedPacketSize, int sessionId);

    void sendGAMETURNSERVERTOAllExceptOne(int sessionId, Card card);

    void doFirstTurnOfFirstRound();
#ifndef NDEBUG
    void checkForCardsCount();
#endif
    void initializeGame();

    void managementGAMETURNCLIENTReceived(int sessionId, Card cardReceived);

    void doTurnReceivedOfFirstRound(std::vector<playerData>::iterator turnReceivedPlayer, Card cardReceived);

    static void newRoundTurn(std::vector<playerData>::iterator currentGamePlayer);

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
