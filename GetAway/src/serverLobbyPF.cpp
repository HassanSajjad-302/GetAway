
#include "serverLobby.hpp"
#include "sati.hpp"

void serverLobby::PF::setGameMain(){
    sati::getInstance()->inputStatement = "2)Close Server 3)Exit\r\n";
    sati::getInstance()->accumulatePrint();
}

void serverLobby::PF::setLobbyMainTwoOrMorePlayers() {
    sati::getInstance()->inputStatement = "1)Start Game 2)Close Server 3)Exit\r\n";
    sati::getInstance()->accumulatePrint();
}
