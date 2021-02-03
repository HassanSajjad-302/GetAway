
#include "serverLobby.hpp"
#include "sati.hpp"

void serverLobby::PF::setGameMain(){
    sati::getInstance()->nonMessageBuffer = "2)Close Server 3)Exit\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}

void serverLobby::PF::setLobbyMainTwoOrMorePlayers() {
    sati::getInstance()->nonMessageBuffer = "1)Start Game 2)Close Server 3)Exit\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}
