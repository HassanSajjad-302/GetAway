#ifndef GETAWAY_SERVERGETAWAY_HPP
#define GETAWAY_SERVERGETAWAY_HPP

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <map>
#include <tuple>
#include "serverSession.hpp"
#include "messageTypeEnums.hpp"
#include "getAwayPData.hpp"
#include "deckSuit.hpp"
#include "asio/io_context.hpp"
class serverLobby;

enum class turnType{
    FIRSTROUNDSPADE,
    FIRSTROUNDANY,
    ROUNDFIRSTTURN,
    ROUNDMIDDLETURN,
    ROUNDLASTTURN,
    THULLA
};

class serverGetAway
{
    serverLobby& lobby;
    const std::map<int, std::tuple<std::string,
    std::unique_ptr<serverSession<serverLobby>>>>& players;

    bool firstRound;
    std::map<deckSuit, std::set<int>> flushedCards;
    deckSuit suitOfTheRound;
    std::vector<getAwayPData> gamePlayersData;
    std::vector<std::tuple<int, Card>> roundTurns; //id and Card

public:
    explicit
    serverGetAway(const std::map<int, std::tuple<std::string,
            std::unique_ptr<serverSession<serverLobby>>>>& gameData_, serverLobby& lobbyManager_);

    void packetReceivedFromNetwork(std::istream &in, int receivedPacketSize, int sessionId);

    void sendGAMETURNSERVERTOAllExceptOne(int sessionId, Card card);

    void doFirstTurnOfFirstRound();
#ifndef NDEBUG
    void checkForCardsCount();
#endif
    void initializeGame();

    void managementGAMETURNCLIENTReceived(int sessionId, Card cardReceived);

    void doTurnReceivedOfFirstRound(std::vector<getAwayPData>::iterator turnReceivedPlayer, Card cardReceived);

    static void newRoundTurn(std::vector<getAwayPData>::iterator currentGamePlayer);

    void Turn(std::vector<getAwayPData>::iterator currentTurnPlayer, Card card);

    void
    performFirstOrMiddleTurn(std::vector<getAwayPData>::iterator currentTurnPlayer, Card card, bool firstTurn);

    void
    performLastOrThullaTurn(std::vector<getAwayPData>::iterator currentTurnPlayer, Card card, bool lastTurn);

    void
    turnCardNumberOfGamePlayerIterator(std::vector<getAwayPData>::iterator turnReceivedPlayer, Card card);

    bool indexGamePlayerDataFromId(int id, int &index);

    std::vector<getAwayPData>::iterator roundKingGamePlayerDataIterator();
};
#endif //GETAWAY_SERVERGETAWAY_HPP
