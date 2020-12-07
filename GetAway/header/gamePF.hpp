//
// Created by hassan on 12/7/20.
//

#ifndef GETAWAY_GAMEPF_HPP
#define GETAWAY_GAMEPF_HPP
#include <set>
#include <mutex>
#include <thread>
#include "boost/asio.hpp"
#include "messageTypeEnums.hpp"
#include "deckSuit.hpp"

class gamePF {

    //Common In Both Lobby And Game
    std::string messageBuffer; //messages
    std::string userIncomingInput;
    std::string inputStatementBuffer; //input

    //Only For Lobby
    std::string playersInLobby;

    //Only For Game
    std::string turnSequence;
    std::string turns;
    std::string waitingForTurn;
    std::string timeLeft;
    std::string cardsString;

public:


    //USED ONLY IN GAME

    //input-statement-functions
    static void setInputStatementHomeTwoInput();
    static void setInputStatementHomeTwoInputAccumulate();

    static void setInputStatementHomeThreeInput();
    static void setInputStatementHomeThreeInputAccumulate();

    static void setInputStatement3(const std::vector<Card>& turnAbleCards_);
    static void setInputStatement3Accumulate(const std::vector<Card>& turnAbleCards_);

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
