//
// Created by hassan on 12/9/20.
//

#include "serverPF.hpp"
#include "sati.hpp"

void serverPF::setHomeMain() {
    sati::getInstance()->inputStatementBuffer = "1)Start Server 2)Exit\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}

void serverPF::setHomeChangeServerName(){
    sati::getInstance()->inputStatementBuffer = "Enter Server Name. Press Enter To Use Default.\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}

void serverPF::setLobbyMainOnePlayer() {
    sati::getInstance()->inputStatementBuffer = "2)Close Server 3)Exit\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}

void serverPF::setGameMain(){
    sati::getInstance()->inputStatementBuffer = "2)Close Server 3)Exit\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}

void serverPF::setLobbyMainTwoOrMorePlayers() {
    sati::getInstance()->inputStatementBuffer = "1)Start Game 2)Close Server 3)Exit\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}
