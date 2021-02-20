//
// Created by hassan on 12/7/20.
//

#include "constants.h"
#include "clientGetAway.hpp"
#include "sati.hpp"

void clientGetAway::PF::accumulateAndPrint() {
    sati::getInstance()->nonMessageBuffer = turnSequence  + "\r\n";
    sati::getInstance()->nonMessageBuffer += turns + "\r\n" + waitingForTurn + "\r\n" + cardsString + "\r\n" +
                                             PF::inputStatementBuffer + "\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}


void clientGetAway::PF::setInputStatementHome2Accumulate(bool clientOnly) {
    clientOnly ?  inputStatementBuffer = "1)Send Message 2)Leave 3)Exit\r\n" :
            inputStatementBuffer = "1)Send Message 2)Close Server 3)Exit\r\n";
    accumulateAndPrint();
}

void clientGetAway::PF::setInputStatementHome3Accumulate(bool clientOnly) {
    clientOnly ? inputStatementBuffer = "1)Send Message 2)Leave 3)Exit 4)Perform Turn\r\n" :
            inputStatementBuffer = "1)Send Message 2)Close Server 3)Exit 4)Perform Turn\r\n";
    accumulateAndPrint();
}

void clientGetAway::PF::setInputStatementHome3R3(const std::vector<Card>& turnAbleCards_) {
    inputStatementBuffer = "Please select one of following:\r\n";
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
            inputStatementBuffer += deckSuitValue::displaySuitType[ca.first] + ": ";
        for(auto t: ca.second){
            inputStatementBuffer += std::to_string(count) + ")" + deckSuitValue::displayCards[t] + " ";
            ++count;
        }
        if(!ca.second.empty())
            inputStatementBuffer += "\t";
    }
    inputStatementBuffer += "\r\n";
}

void clientGetAway::PF::setInputStatementHome3R3Accumulate(const std::vector<Card>& turnAbleCards_) {
    setInputStatementHome3R3(turnAbleCards_);
    accumulateAndPrint();
}
//other
void clientGetAway::PF::setTurnSequence(const std::map<int, std::string> &gamePlayer_, const std::vector<int>& turnSequence_) {
    turnSequence = "Turn Sequence: ";
    for(auto& t: turnSequence_){
        turnSequence += gamePlayer_.find(t)->second + "\t";
    }
    turnSequence += "\r\n";
}

void clientGetAway::PF::setRoundTurns(
        const std::vector<std::tuple<int, Card>>& roundTurns, const std::map<int, std::string>& gamePlayers) {
    turns.clear();
    for(auto& tu: roundTurns){
        turns += gamePlayers.find(std::get<0>(tu))->second;
        assert((std::get<1>(tu).cardNumber >=0 && std::get<1>(tu).cardNumber < constants::SUITSIZE)
               && "CardNumber not in range in setRoundTurns");
        turns += ": " + deckSuitValue::displaySuitType[(int) std::get<1>(tu).suit] + " "
                 + deckSuitValue::displayCards[std::get<1>(tu).cardNumber] + "\r\n";
    }
}

void clientGetAway::PF::setRoundTurnsAccumulate(
        const std::vector<std::tuple<int, Card>>& roundTurns, const std::map<int, std::string>& gamePlayers) {
    setRoundTurns(roundTurns, gamePlayers);
    accumulateAndPrint();
}

void clientGetAway::PF::setWaitingForTurn(const std::vector<int> &waitingplayersId,
                           const std::map<int, std::string> &gamePlayers) {
    waitingForTurn = "Waiting For Turn:\r\n";
    for(auto&waitingId: waitingplayersId){
        waitingForTurn += gamePlayers.find(waitingId)->second + "\r\n";
    }
}

void clientGetAway::PF::setCards(const std::map<deckSuit, std::set<int>> &cards) {
    cardsString = "Your Cards\r\n";
    for(auto& ca: cards){
        cardsString += deckSuitValue::displaySuitType[(int) ca.first] + ": ";
        for(auto it = ca.second.begin(); it != ca.second.end(); ++it){
            ++it;
            if(it == ca.second.end()){
                --it;
                cardsString += deckSuitValue::displayCards[ *it % constants::SUITSIZE ];
            }else{
                --it;
                cardsString += deckSuitValue::displayCards[ *it % constants::SUITSIZE ] + ", ";
            }
        }
        cardsString += "\r\n";
    }
}