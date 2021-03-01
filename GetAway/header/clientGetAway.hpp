#ifndef GETAWAY_CLIENTGETAWAY_HPP
#define GETAWAY_CLIENTGETAWAY_HPP

#include <map>
#include <set>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include "serverSession.hpp"
#include "messageTypeEnums.hpp"
#include "terminalInputBase.hpp"
#include "inputType.h"
#include "deckSuit.hpp"
#include "asio/io_context.hpp"

class clientLobby;
enum whoTurned{
    CLIENT,
    RECEIVED,
};
// Represents the shared server state
class clientGetAway : terminalInputBase {
    class PF {
        //static inline std::string inputStatementBuffer;
        static inline std::string turnSequence;
        static inline std::string cardsString;
        static inline std::string waitingForTurn;
        static inline std::string turns;
        static inline void accumulateAndPrint();
    public:
        //input-statement-functions
        static void setInputStatementHome2Accumulate(bool clientOnly);

        static void setInputStatementHome3Accumulate(bool clientOnly);

        static void setInputStatementHome3R3(const std::vector<Card>& turnAbleCards_);
        static void setInputStatementHome3R3Accumulate(const std::vector<Card>& turnAbleCards_);

        static void setTurnSequence(const std::map<int, std::string>& gamePlayer_, const std::vector<int>& turnSequence_);

        static void
        setRoundTurns(const std::vector<std::tuple<int, Card>> &roundTurns,
                      const std::map<int, std::string> &gamePlayers);

        static void setRoundTurnsAccumulate(const std::vector<std::tuple<int, Card>> &roundTurns,
                                            const std::map<int, std::string> &gamePlayers);

        static void setWaitingForTurn(const std::vector<int>& waitingplayersId, const std::map<int, std::string>& gamePlayers);

        static void setCards(const std::map<deckSuit, std::set<int>>& cardsMap);
    };

    clientLobby& lobbyManager;
    //asio::io_context& io;
    const std::string& playerName;
    const std::map<int, std::string>& players;
    int myId = 0;
    //std::shared_ptr<serverSession<clientLobbyManager>> clientLobbySession;
    inputType inputTypeExpected;

    void input(std::string inputString, inputType inputReceivedType) override;

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
    bool gameFinished = false;
    bool clientOnly;
public:
    explicit
    clientGetAway(clientLobby &lobbyManager_, const std::string& playerName_,
                  const std::map<int, std::string>& players_, std::istream& in, int myId_, bool clientOnly_);

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

    void setBaseAndInputTypeFromclientChatMessage();
};

#endif //GETAWAY_CLIENTGETAWAY_HPP
