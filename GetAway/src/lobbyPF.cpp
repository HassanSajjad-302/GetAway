//
// Created by hassan on 12/7/20.
//

#include "lobbyPF.hpp"
#include "sati.hpp"

//USED ONLY FOR LOBBY
//input-statement functions
void lobbyPF::setInputStatementHome() {
    sati::getInstance()->inputStatementBuffer = "1)Send Message 2)Leave 3)Exit\r\n";
}
void lobbyPF::setInputStatementHomeAccumulate() {
    sati::getInstance()->inputStatementBuffer = "1)Send Message 2)Leave 3)Exit\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}

//others

void lobbyPF::addOrRemovePlayer(const std::map<int, std::string> &gamePlayer_) {
    sati::getInstance()->playersInLobby = "Players in Lobby Are: ";
    for(auto& player: gamePlayer_){
        sati::getInstance()->playersInLobby += player.second + "\t";
    }
    sati::getInstance()->playersInLobby += "\r\n";
}

void lobbyPF::addOrRemovePlayerAccumulate(const std::map<int, std::string> &gamePlayer_) {
    addOrRemovePlayer(gamePlayer_);
    sati::getInstance()->accumulateBuffersAndPrint();
}