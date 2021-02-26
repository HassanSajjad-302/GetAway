#include "constants.h"
#include "clientBluff.hpp"
#include "sati.hpp"

void Bluff::clientBluff::PF::accumulateAndPrint() {
    sati::getInstance()->nonMessageBuffer = turnSequence  + "\r\n";
    sati::getInstance()->nonMessageBuffer += turns + "\r\n" + waitingForTurn + "\r\n" + cardsString + "\r\n" +
                                             PF::inputStatementBuffer + "\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}

void Bluff::clientBluff::PF::promptSimpleNonTurnInput(bool clientOnly) {
    clientOnly ?  inputStatementBuffer = "1)Send Message 2)Leave 3)Exit\r\n" :
            inputStatementBuffer = "1)Send Message 2)Close Server 3)Exit\r\n";
    accumulateAndPrint();
}

void Bluff::clientBluff::PF::promptFirstTurn(bool clientOnly){
    clientOnly ? inputStatementBuffer = "1)Send Message 2)Leave 3)Exit 4)Play Cards\r\n" :
            inputStatementBuffer = "1)Send Message 2)Close Server 3)Exit 4)Play Cards\r\n";
    accumulateAndPrint();
}

void Bluff::clientBluff::PF::promptFirstTurnAccumulate(bool clientOnly){
   promptFirstTurn(clientOnly);
    accumulateAndPrint();
}

void Bluff::clientBluff::PF::promptFirsTurnSelectDeckSuitAccumulate(){
    inputStatementBuffer = "Please select the suit for the round from following\r\n"
                           "1)CLUB 2)HEART 3)SPADE 4)DIAMOND";
    accumulateAndPrint();
}

void Bluff::clientBluff::PF::promptNormalTurnAccumulate(bool clientOnly){
    clientOnly ? inputStatementBuffer = "1)Send Message 2)Leave 3)Exit 4)Play Cards 5)Check 6)Pass\r\n" :
            inputStatementBuffer = "1)Send Message 2)Close Server 3)Exit 4)Play Cards 5)Check 6)Pass\r\n";
    accumulateAndPrint();
}

void Bluff::clientBluff::PF::promptAndInputWaitingForCardsAccumulate(bool clientOnly) {
    promptFirstTurn(clientOnly);
    inputStatementBuffer += "Waiting For Cards Of Last Turn\r\n";
    accumulateAndPrint();
}

void Bluff::clientBluff::PF::promptFirstTurnOrNormalTurnSelectCards(const std::map<deckSuit, std::set<int>>& turnAbleCards_) {
    inputStatementBuffer = "Please enter card numbers separated by spaces:\r\n";
    int count = 1;
    for(auto& ca: turnAbleCards_){
        if(!ca.second.empty())
            inputStatementBuffer += deckSuitValue::displaySuitType[(int)ca.first] + ": ";
        for(auto t: ca.second){
            inputStatementBuffer += std::to_string(count) + ")" + deckSuitValue::displayCards[t] + " ";
            ++count;
        }
        if(!ca.second.empty())
            inputStatementBuffer += "\t";
    }
    inputStatementBuffer += "\r\n";
    accumulateAndPrint();
}
//other
void Bluff::clientBluff::PF::setTurnSequence(const std::map<int, std::string> &gamePlayer_, const std::vector<int>& turnSequence_) {
    turnSequence = "Turn Sequence: ";
    for(auto& t: turnSequence_){
        turnSequence += gamePlayer_.find(t)->second + "\t";
    }
    turnSequence += "\r\n";
}

void Bluff::clientBluff::PF::setRoundTurns(
        const std::vector<std::tuple<int, Bluff::bluffTurn>>& roundTurns, const std::map<int, std::string>& gamePlayers) {
    turns.clear();
    for(auto& tu: roundTurns){
        std::string playerName = gamePlayers.find(std::get<0>(tu))->second;
        auto& bluffT = std::get<1>(tu);
        turns += playerName + ": ";
        switch(bluffT.turn){
            case Bluff::turnType::FIRST:{
                turns += std::to_string(bluffT.cardCount) + " " +
                        deckSuitValue::displaySuitType[(int) bluffT.firstTurnSuit];
                break;
            }
            case Bluff::turnType::NORMAL:{
                turns += std::to_string(bluffT.cardCount) + " " + "MORE";
                break;
            }
            case Bluff::turnType::CHECK:{
                turns += "CHECK\r\n";
                for(auto& ca: bluffT.checkTurn_LastTurnCards){
                    turns += deckSuitValue::displaySuitType[(int) ca.suit] + " " +
                            deckSuitValue::displayCards[ca.cardNumber] + " ";
                }
                break;
            }
            case Bluff::turnType::PASS:{
                turns += "PASS";
                break;
            }
        }
        turns += "\r\n";
    }
}

void Bluff::clientBluff::PF::setRoundTurnsAccumulate(
        const std::vector<std::tuple<int, Bluff::bluffTurn>>& roundTurns, const std::map<int, std::string>& gamePlayers) {
    setRoundTurns(roundTurns, gamePlayers);
    accumulateAndPrint();
}

void Bluff::clientBluff::PF::setWaitingForTurn(int waitingplayerId,
                                               const std::map<int, std::string> &gamePlayers) {
    waitingForTurn = "Waiting For Turn:\r\n";
    waitingForTurn += gamePlayers.find(waitingplayerId)->second + "\r\n";
}

void Bluff::clientBluff::PF::setCards(const std::map<deckSuit, std::set<int>> &cards) {
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