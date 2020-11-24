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
            if(!gameStarted){
                accumulateBuffersAndPrintWithOutLockLobby();
            }else{
                accumulateBuffersAndPrintWithOutLockGame();
            }
        }else if(c == 10 || c == 13){ //cr pressed
            std::lock_guard<std::mutex> lok(m.get());
            if(handlerAssigned){
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
            if(!gameStarted){
                accumulateBuffersAndPrintWithLockLobby();
            }else{
                accumulateBuffersAndPrintWithLockGame();
            }
        }
    }
}

void sati::setReceiveInputTypeAndGameStarted(inputType nextReceiveInputType, bool gameStarted_) {
    std::lock_guard<std::mutex> lok(m.get());
    gameStarted = gameStarted_;
    receiveInputType = nextReceiveInputType;
    handlerAssigned = true;
}

void sati::accumulateBuffersAndPrintWithLockLobby() {
    std::string toPrint = messageBuffer + playersInLobby + inputStatementBuffer;
    {
        std::lock_guard lock(m.get());
        toPrint += userIncomingInput;
    }
    system("clear");
    std::cout<<toPrint;
}

void sati::accumulateBuffersAndPrintWithOutLockLobby() {
    std::string toPrint = messageBuffer + playersInLobby + inputStatementBuffer;
    toPrint += userIncomingInput;
    system("clear");
    std::cout<<toPrint;
}

void sati::accumulateBuffersAndPrintWithLockGame() {

}

void sati::accumulateBuffersAndPrintWithOutLockGame() {

}

void sati::lobbyAccumulatePrint() {
    accumulateBuffersAndPrintWithLockLobby();
}

void sati::gameAccumulatePrint() {
    accumulateBuffersAndPrintWithLockGame();
}


void sati::printExitMessage(std::string message) {
    std::lock_guard<std::mutex> lockGuard(m);
    std::cout<<message<<std::endl;
}

void sati::setInputType(inputType nextReceiveInputType) {
    std::lock_guard<std::mutex> lock{m.get()};
    receiveInputType = nextReceiveInputType;
    handlerAssigned = true;
}

void sati::setBase(inputRead *base_) {
    base = base_;
}



//For Lobby


void sati::addMessagePrint(const std::string &playerName, const std::string &message) {
    messageBuffer += playerName + " : " + message + "\r\n\n";
}
void sati::addMessageAccumulatePrint(const std::string& playerName, const std::string& message) {
    messageBuffer += playerName + " : " + message + "\r\n\n";
    accumulateBuffersAndPrintWithLockLobby();
}


void sati::addOrRemovePlayerLobbyPrint(const std::map<int, std::string> &gamePlayer_) {
    playersInLobby = "Players in Lobby Are: ";
    for(auto& player: gamePlayer_){
        playersInLobby += player.second + "\t";
    }
    playersInLobby += "\r\n\n";
}
void sati::addOrRemovePlayerLobbyAccumulatePrint(const std::map<int, std::string> &gamePlayer_) {
    playersInLobby = "Players in Lobby Are: ";
    for(auto& player: gamePlayer_){
        playersInLobby += player.second + "\t";
    }
    playersInLobby += "\r\n\n";
    accumulateBuffersAndPrintWithLockLobby();
}

void sati::setInputStatementHomeLobbyPrint() {
    inputStatementBuffer = "1)Send Message 2)Exit\r\n";
}
void sati::setInputStatementHomeLobbyAccumulatePrint() {
    inputStatementBuffer = "1)Send Message 2)Exit\r\n";
    accumulateBuffersAndPrintWithLockLobby();
}

void sati::setInputStatementMessagePrint() {
    inputStatementBuffer = "Please Type The Message \r\n\n";
}

void sati::setInputStatementMessageAccumulatePrint() {
    inputStatementBuffer = "Please Type The Message \r\n\n";
    accumulateBuffersAndPrintWithLockLobby();
}



//For Game

//input statement
void sati::setInputStatementHomeTwoInputGamePrint() {
    inputStatementBuffer = "1)Send Message 2)Exit\r\n";
}

void sati::setInputStatementHomeTwoInputGameAccumulatePrint() {
    inputStatementBuffer = "1)Send Message 2)Exit\r\n";
    accumulateBuffersAndPrintWithLockLobby();
}

void sati::setInputStatementHomeThreeInputGamePrint() {
    inputStatementBuffer = "1)Send Message 2)Exit 3)Perform Turn\r\n\n";
}

void sati::setInputStatementHomeThreeInputGameAccumulatePrint() {
    inputStatementBuffer = "1)Send Message 2)Exit 3)Perform Turn\r\n\n";
    accumulateBuffersAndPrintWithLockLobby();
}

//other
void sati::setTurnSequenceGamePrint(const std::map<int, std::string> &gamePlayer_, const std::vector<int>& turnSequence_) {
    turnSequence = "Turn Sequence: ";
    for(auto& t: turnSequence_){
        turnSequence += gamePlayer_.find(t)->second + "\t";
    }
    turnSequence += "\r\n\n";
}

void sati::setTurnSequenceGameAccumulatePrint(const std::map<int, std::string> &gamePlayer_, const std::vector<int>& turnSequence_) {
    turnSequence = "Turn Sequence: ";
    for(auto& t: turnSequence_){
        turnSequence += gamePlayer_.find(t)->second + "\t";
    }
    turnSequence += "\r\n\n";
    accumulateBuffersAndPrintWithLockGame();
}

void sati::addTurnGamePrint(const std::string& playerName, int cardNumber) {
    turns += playerName;
    assert(cardNumber/13 >=0 && cardNumber/13 <=4 && "CardNumber not in range in addTurnGamePrint");
    turns += ": " + deckSuitValue::literal[cardNumber/13] + "\r\n";
}

void sati::addTurnGameAccumulatePrint(const std::string& playerName, int cardNumber) {
    turns += playerName;
    assert(cardNumber/13 >=0 && cardNumber/13 <=4 && "CardNumber not in range in addTurnGamePrint");
    turns += ": " + deckSuitValue::literal[cardNumber/13] + "\r\n";
    accumulateBuffersAndPrintWithLockGame();
}

void sati::clearTurnGamePrint() {
    turns.clear();
}

void sati::clearTurnGameAccumulatePrint() {
    turns.clear();
    accumulateBuffersAndPrintWithLockGame();
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
    waitingForTurn = "Waiting For Turn:\r\n";
    for(auto&waitingId: waitingPlayersId){
        waitingForTurn += gamePlayers.find(waitingId)->second + "\r\n";
    }
    accumulateBuffersAndPrintWithLockGame();
}

void sati::setTimeLeftGamePrint(int seconds) {
    timeLeft = "Time Left" + std::to_string(seconds) + "s\r\n";
}

void sati::setTimeLeftGameAccumulatePrint(int seconds) {
    timeLeft = "Time Left" + std::to_string(seconds) + "s\r\n";
    accumulateBuffersAndPrintWithLockGame();
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
    accumulateBuffersAndPrintWithLockGame();
}