//
// Created by hassan on 12/7/20.
//

#ifndef GETAWAY_GAMEPF_HPP
#define GETAWAY_GAMEPF_HPP
#include <set>
#include <vector>
#include <mutex>
#include <thread>
#include "messageTypeEnums.hpp"
#include "deckSuit.hpp"

class gamePF {
public:
    //input-statement-functions
    static void setInputStatementHome2();
    static void setInputStatementHome2Accumulate();

    static void setInputStatementHome3();
    static void setInputStatementHome3Accumulate();

    static void setInputStatementHome3R3(const std::vector<Card>& turnAbleCards_);
    static void setInputStatementHome3R3Accumulate(const std::vector<Card>& turnAbleCards_);

    static void setTurnSequence(const std::map<int, std::string>& gamePlayer_, const std::vector<int>& turnSequence_);
    static void setTurnSequenceAccumulate(const std::map<int, std::string>& gamePlayer_, const std::vector<int>& turnSequence_);

    static void
    setRoundTurns(const std::vector<std::tuple<int, Card>> &roundTurns,
                           const std::map<int, std::string> &gamePlayers);

    static void setRoundTurnsAccumulate(const std::vector<std::tuple<int, Card>> &roundTurns,
                                          const std::map<int, std::string> &gamePlayers);

    static void clearTurn();
    static void clearTurnAccumulate();

    static void setWaitingForTurn(const std::vector<int>& waitingplayersId, const std::map<int, std::string>& gamePlayers);
    static void setWaitingForTurnAccumulate(const std::vector<int>& waitingPlayersId, const std::map<int, std::string>& gamePlayers);

    static void setTimeLeft(int seconds);
    static void setTimeLeftAccumulate(int seconds);

    static void setCards(const std::map<deckSuit, std::set<int>>& cardsMap);
    static void setCardsAccumulate(const std::map<deckSuit, std::set<int>>& cardsMap);
};


#endif //GETAWAY_GAMEPF_HPP
