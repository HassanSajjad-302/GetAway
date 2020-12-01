//
// Created by hassan on 11/6/20.
//

#include <set>
#include <iostream>
#include <utility>
#include "sati.hpp"
#include <functional>
#include "deckSuit.hpp"

sati& sati::getInstanceFirstTime(net::io_context& io_, std::mutex& mut) {
    static sati s{io_, mut};
    oneInstanceOnly = &s;
    return s;
}

sati* sati::getInstance(){
    return oneInstanceOnly;
}

sati::sati(net::io_context &io_, std::mutex& mut) : io{io_}, m{mut}{

}


void sati::setInputType(inputType nextReceiveInputType) {
    std::lock_guard<std::mutex> lock{m.get()};
    receiveInputType = nextReceiveInputType;
    handlerAssigned = true;
}

void sati::printExitMessage(const std::string& message) {
    std::lock_guard<std::mutex> lockGuard(m);
    std::cout<<message<<std::endl;
}

void sati::setBase(inputRead *base_) {
    base = base_;
}

void sati::operator()() {
    system("stty raw");
    while(true){
        int c = getchar();
        if(c == 3) //ctrl + c
        {
            system("stty cooked");
            exit(0);
        }else if( c == 127){
            std::lock_guard<std::mutex> lok(m.get());
            if(!userIncomingInput.empty()){
                userIncomingInput.pop_back();
            }
            accumulateBuffersAndPrint(true);
        }else if(c == 10 || c == 13){ //cr pressed
            std::lock_guard<std::mutex> lok(m.get());
            if(handlerAssigned && (base != nullptr)){
                net::post(io, [handler = base, expectedInput = receiveInputType,
                        str = std::move(userIncomingInput)](){
                    handler->input(str, expectedInput);
                });
                handlerAssigned = false;
                userIncomingInput.clear();
            }
        }
        else{
            userIncomingInput += c;
            accumulateBuffersAndPrint(true);
        }
    }
}

void sati::setReceiveInputTypeAndGameStarted(inputType nextReceiveInputType, bool gameStarted_) {
    std::lock_guard<std::mutex> lok(m.get());
    gameStarted = gameStarted_;
    receiveInputType = nextReceiveInputType;
    handlerAssigned = true;
}

void sati::accumulateBuffersAndPrint(bool lock) {
    std::string toPrint;
    if(!messageBuffer.empty()){
        toPrint = messageBuffer + "\r\n";
    }
    if(!gameStarted){
        toPrint += playersInLobby  + "\r\n" + inputStatementBuffer + "\r\n";
    }else{
        toPrint += turnSequence  + "\r\n";
        toPrint += turns + "\r\n" + waitingForTurn + "\r\n" + cardsString + "\r\n" + inputStatementBuffer + "\r\n";
    }
    if(lock){
        std::lock_guard lockGuard(m.get());
        toPrint += userIncomingInput;
    }
    else{
        toPrint += userIncomingInput;
    }
    system("clear");
    std::cout<<toPrint;
}

void sati::accumulatePrint(){
    accumulateBuffersAndPrint(true);
}

void sati::setInputStatementMessagePrint() {
    inputStatementBuffer = "Please Type The Message \r\n";
}

void sati::setInputStatementMessageAccumulatePrint() {
    inputStatementBuffer = "Please Type The Message \r\n";
    accumulateBuffersAndPrint(true);
}

void sati::addMessagePrint(const std::string &playerName, const std::string &message) {
    messageBuffer += playerName + " : " + message + "\r\n";
}
void sati::addMessageAccumulatePrint(const std::string& playerName, const std::string& message) {
    messageBuffer += playerName + " : " + message + "\r\n";
    accumulateBuffersAndPrint(true);
}


//USED ONLY FOR LOBBY
//input-statement functions
void sati::setInputStatementHomeLobbyPrint() {
    inputStatementBuffer = "1)Send Message 2)Exit\r\n";
}
void sati::setInputStatementHomeLobbyAccumulatePrint() {
    inputStatementBuffer = "1)Send Message 2)Exit\r\n";
    accumulateBuffersAndPrint(true);
}

//others

void sati::addOrRemovePlayerLobbyPrint(const std::map<int, std::string> &gamePlayer_) {
    playersInLobby = "Players in Lobby Are: ";
    for(auto& player: gamePlayer_){
        playersInLobby += player.second + "\t";
    }
    playersInLobby += "\r\n";
}
void sati::addOrRemovePlayerLobbyAccumulatePrint(const std::map<int, std::string> &gamePlayer_) {
    playersInLobby = "Players in Lobby Are: ";
    for(auto& player: gamePlayer_){
        playersInLobby += player.second + "\t";
    }
    playersInLobby += "\r\n";
    accumulateBuffersAndPrint(true);
}


//For Game

//input statement
void sati::setInputStatementHomeTwoInputGamePrint() {
    inputStatementBuffer = "1)Send Message 2)Exit\r\n";
}

void sati::setInputStatementHomeTwoInputGameAccumulatePrint() {
    inputStatementBuffer = "1)Send Message 2)Exit\r\n";
    accumulateBuffersAndPrint(true);
}

void sati::setInputStatementHomeThreeInputGamePrint() {
    inputStatementBuffer = "1)Send Message 2)Exit 3)Perform Turn\r\n";
}

void sati::setInputStatementHomeThreeInputGameAccumulatePrint() {
    inputStatementBuffer = "1)Send Message 2)Exit 3)Perform Turn\r\n";
    accumulateBuffersAndPrint(true);
}

void sati::setInputStatement3GamePrint(const std::set<int>& cards_, int deckSuitType) {
    inputStatementBuffer = "Please select one of following:\r\n";
    inputStatementBuffer += deckSuitValue::displayCards[deckSuitType] + ": ";
    int count = 1;
    for(auto t: cards_){
        inputStatementBuffer += std::to_string(count) + ")" + deckSuitValue::displayCards[t] + " ";
        ++count;
    }
    inputStatementBuffer += "\r\n";
}

void sati::setInputStatement3GameAccumulatePrint(const std::set<int>& cards_, int deckSuitType) {
    setInputStatement3GamePrint(cards_, deckSuitType);
    accumulateBuffersAndPrint(true);
}

void sati::setInputStatement3GamePrint(const std::map<int, std::set<int>>& cards_) {
    inputStatementBuffer = "Please select one of following:\r\n";
    int count = 1;
    for(auto& ca: cards_){
        inputStatementBuffer += deckSuitValue::displayCards[ca.first] + ": ";
        for(auto t: ca.second){
            inputStatementBuffer += std::to_string(count) + ")" + deckSuitValue::displayCards[t] + " ";
            ++count;
        }
        inputStatementBuffer += "\t";
    }
    inputStatementBuffer += "\r\n";
}

void sati::setInputStatement3GameAccumulatePrint(const std::map<int, std::set<int>>& cards_) {
    inputStatementBuffer = "Please select one of following:\r\n";
    accumulateBuffersAndPrint(true);
}


//other
void sati::setTurnSequenceGamePrint(const std::map<int, std::string> &gamePlayer_, const std::vector<int>& turnSequence_) {
    turnSequence = "Turn Sequence: ";
    for(auto& t: turnSequence_){
        turnSequence += gamePlayer_.find(t)->second + "\t";
    }
    turnSequence += "\r\n";
}

void sati::setTurnSequenceGameAccumulatePrint(const std::map<int, std::string> &gamePlayer_, const std::vector<int>& turnSequence_) {
    setTurnSequenceGamePrint(gamePlayer_, turnSequence_);
    accumulateBuffersAndPrint(true);
}

void sati::setRoundTurnsGamePrint(
        const std::vector<std::tuple<int, int>>& roundTurns, const std::map<int, std::string>& gamePlayers) {
    for(auto& tu: roundTurns){
        assert((std::get<0>(tu)/13) == (int)deckSuit::SPADE && "Card Receved Not In Spade");
        turns += gamePlayers.find(std::get<1>(tu))->second;
        assert(std::get<0>(tu)/13 >=0 && std::get<0>(tu)/13 <=4 && "CardNumber not in range in setRoundTurnsGamePrint");
        turns += ": " + deckSuitValue::literal[std::get<1>(tu)/13] + " "
                + deckSuitValue::displayCards[std::get<1>(tu)] + "\r\n";
    }
}

void sati::setRoundTurnsGameAccumulatePrint(
        const std::vector<std::tuple<int, int>>& roundTurns, const std::map<int, std::string>& gamePlayers) {
    setRoundTurnsGamePrint(roundTurns, gamePlayers);
    accumulateBuffersAndPrint(true);
}

void sati::clearTurnGamePrint() {
    turns.clear();
}

void sati::clearTurnGameAccumulatePrint() {
    turns.clear();
    accumulateBuffersAndPrint(true);
}

void sati::setWaitingForTurnGamePrint(const std::vector<int> &waitingplayersId,
                                      const std::map<int, std::string> &gamePlayers) {
    waitingForTurn = "Waiting For Turn:\r\n";
    for(auto&waitingId: waitingplayersId){
        waitingForTurn += gamePlayers.find(waitingId)->second + "\r\n";
    }
}

void sati::setWaitingForTurnGameAccumulatePrint(const std::vector<int> &waitingPlayersId,
                                                const std::map<int, std::string> &gamePlayers) {
    setWaitingForTurnGamePrint(waitingPlayersId, gamePlayers);
    accumulateBuffersAndPrint(true);
}

void sati::setTimeLeftGamePrint(int seconds) {
    timeLeft = "Time Left" + std::to_string(seconds) + "s\r\n";
}

void sati::setTimeLeftGameAccumulatePrint(int seconds) {
    timeLeft = "Time Left" + std::to_string(seconds) + "s\r\n";
    accumulateBuffersAndPrint(true);
}

void sati::setCardsGamePrint(const std::map<int, std::set<int>> &cards) {
    cardsString = "Your Cards\r\n";
    for(auto& ca: cards){
        cardsString += deckSuitValue::literal[(int) ca.first] + ": ";
        for(auto it = ca.second.begin(); it != ca.second.end(); ++it){
            ++it;
            if(it == ca.second.end()){
                --it;
                cardsString += deckSuitValue::displayCards[ *it % 13 ];
            }else{
                --it;
                cardsString += deckSuitValue::displayCards[ *it % 13 ] + ", ";
            }
        }
        cardsString += "\r\n";
    }
}

void sati::setCardsGameAccumulatePrint(const std::map<int, std::set<int>> &cards) {
    setCardsGamePrint(cards);
    accumulateBuffersAndPrint(true);
}



