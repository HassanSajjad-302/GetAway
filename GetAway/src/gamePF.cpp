//
// Created by hassan on 12/7/20.
//

#include "constants.h"
#include "gamePF.hpp"
#ifdef ANDROID
#include "satiAndroid.hpp"
#else
#include "sati.hpp"
#endif


//For Game

//input statement
void gamePF::setInputStatementHome2() {
    sati::getInstance()->inputStatementBuffer = "1)Send Message 2)Exit\r\n";
}

void gamePF::setInputStatementHome2Accumulate() {
    sati::getInstance()->inputStatementBuffer = "1)Send Message 2)Exit\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}

void gamePF::setInputStatementHome3() {
    sati::getInstance()->inputStatementBuffer = "1)Send Message 2)Exit 3)Perform Turn\r\n";
}

void gamePF::setInputStatementHome3Accumulate() {
    sati::getInstance()->inputStatementBuffer = "1)Send Message 2)Exit 3)Perform Turn\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}

void gamePF::setInputStatementHome3R3(const std::vector<Card>& turnAbleCards_) {
    sati::getInstance()->inputStatementBuffer = "Please select one of following:\r\n";
    int count = 1;
    std::map<int, std::set<int>> cards;
    for(auto c: turnAbleCards_){
        auto it = cards.find((int)c.suit);
        if(it == cards.end()){
            std::set<int> s;
            s.emplace(c.cardNumber);
            cards.emplace((int)c.suit, s);
        }else{
            it->second.emplace(c.cardNumber);
        }
    }
    for(auto& ca: cards){
        if(!ca.second.empty())
            sati::getInstance()->inputStatementBuffer += deckSuitValue::displaySuitType[ca.first] + ": ";
        for(auto t: ca.second){
            sati::getInstance()->inputStatementBuffer += std::to_string(count) + ")" + deckSuitValue::displayCards[t] + " ";
            ++count;
        }
        if(!ca.second.empty())
            sati::getInstance()->inputStatementBuffer += "\t";
    }
    sati::getInstance()->inputStatementBuffer += "\r\n";
}

void gamePF::setInputStatementHome3R3Accumulate(const std::vector<Card>& turnAbleCards_) {
    setInputStatementHome3R3(turnAbleCards_);
    sati::getInstance()->accumulateBuffersAndPrint();
}
//other
void gamePF::setTurnSequence(const std::map<int, std::string> &gamePlayer_, const std::vector<int>& turnSequence_) {
    sati::getInstance()->turnSequence = "Turn Sequence: ";
    for(auto& t: turnSequence_){
        sati::getInstance()->turnSequence += gamePlayer_.find(t)->second + "\t";
    }
    sati::getInstance()->turnSequence += "\r\n";
}

void gamePF::setTurnSequenceAccumulate(const std::map<int, std::string> &gamePlayer_, const std::vector<int>& turnSequence_) {
    setTurnSequence(gamePlayer_, turnSequence_);
    sati::getInstance()->accumulateBuffersAndPrint();
}

void gamePF::setRoundTurns(
        const std::vector<std::tuple<int, Card>>& roundTurns, const std::map<int, std::string>& gamePlayers) {
    sati::getInstance()->turns.clear();
    for(auto& tu: roundTurns){
        sati::getInstance()->turns += gamePlayers.find(std::get<0>(tu))->second;
        assert((std::get<1>(tu).cardNumber >=0 && std::get<1>(tu).cardNumber < constants::SUITSIZE)
               && "CardNumber not in range in setRoundTurns");
        sati::getInstance()->turns += ": " + deckSuitValue::displaySuitType[(int) std::get<1>(tu).suit] + " "
                 + deckSuitValue::displayCards[std::get<1>(tu).cardNumber] + "\r\n";
    }
}

void gamePF::setRoundTurnsAccumulate(
        const std::vector<std::tuple<int, Card>>& roundTurns, const std::map<int, std::string>& gamePlayers) {
    setRoundTurns(roundTurns, gamePlayers);
    sati::getInstance()->accumulateBuffersAndPrint();
}

void gamePF::clearTurn() {
    sati::getInstance()->turns.clear();
}

void gamePF::clearTurnAccumulate() {
    sati::getInstance()->turns.clear();
    sati::getInstance()->accumulateBuffersAndPrint();
}

void gamePF::setWaitingForTurn(const std::vector<int> &waitingplayersId,
                                        const std::map<int, std::string> &gamePlayers) {
    sati::getInstance()->waitingForTurn = "Waiting For Turn:\r\n";
    for(auto&waitingId: waitingplayersId){
        sati::getInstance()->waitingForTurn += gamePlayers.find(waitingId)->second + "\r\n";
    }
}

void gamePF::setWaitingForTurnAccumulate(const std::vector<int> &waitingPlayersId,
                                                  const std::map<int, std::string> &gamePlayers) {
    setWaitingForTurn(waitingPlayersId, gamePlayers);
    sati::getInstance()->accumulateBuffersAndPrint();
}

void gamePF::setTimeLeft(int seconds) {
    sati::getInstance()->timeLeft = "Time Left" + std::to_string(seconds) + "s\r\n";
}

void gamePF::setTimeLeftAccumulate(int seconds) {
    sati::getInstance()->timeLeft = "Time Left" + std::to_string(seconds) + "s\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}

void gamePF::setCards(const std::map<deckSuit, std::set<int>> &cards) {
    sati::getInstance()->cardsString = "Your Cards\r\n";
    for(auto& ca: cards){
        sati::getInstance()->cardsString += deckSuitValue::displaySuitType[(int) ca.first] + ": ";
        for(auto it = ca.second.begin(); it != ca.second.end(); ++it){
            ++it;
            if(it == ca.second.end()){
                --it;
                sati::getInstance()->cardsString += deckSuitValue::displayCards[ *it % constants::SUITSIZE ];
            }else{
                --it;
                sati::getInstance()->cardsString += deckSuitValue::displayCards[ *it % constants::SUITSIZE ] + ", ";
            }
        }
        sati::getInstance()->cardsString += "\r\n";
    }
}

void gamePF::setCardsAccumulate(const std::map<deckSuit, std::set<int>> &cards) {
    setCards(cards);
    sati::getInstance()->accumulateBuffersAndPrint();
}



