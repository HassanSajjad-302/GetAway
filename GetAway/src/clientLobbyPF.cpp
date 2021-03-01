
#include "clientLobby.hpp"
#include "sati.hpp"

void clientLobby::PF::addOrRemovePlayerAccumulate(const std::map<int, std::string> &gamePlayer_) {
    playersInLobby = "Players in Lobby Are: ";
    for(auto& player: gamePlayer_){
        playersInLobby += player.second + "\t";
    }
    playersInLobby += "\r\n";
    sati::getInstance()->nonMessageBuffer = playersInLobby + "\r\n";
    sati::getInstance()->accumulatePrint();
}

void clientLobby::PF::setInputStatementHomeAccumulate() {
    sati::getInstance()->nonMessageBuffer = playersInLobby  + "\r\n";
    sati::getInstance()->accumulatePrint();
}
