//
// Created by hassan on 12/9/20.
//

#include "serverPF.hpp"
#include "sati.hpp"

void serverPF::setHomeMain() {
    sati::getInstance()->inputStatementBuffer = "1)Start Server 2)Change Port 3)Exit\r\n";
    sati::getInstance()->accumulateBuffersAndPrint(true);
}

void serverPF::setHomeChangePort(){
    sati::getInstance()->inputStatementBuffer = "Enter Port Number\r\n";
    sati::getInstance()->accumulateBuffersAndPrint(true);
}

void serverPF::setErrorMessageWrongPortNumber() {
    sati::getInstance()->inputStatementBuffer = "Entered Port Number Could Not Be Validated\r\n";
    sati::getInstance()->accumulateBuffersAndPrint(true);
}

void serverPF::setLobbyMainOnePlayer() {
    sati::getInstance()->inputStatementBuffer = "1)Exit\r\n";
    sati::getInstance()->accumulateBuffersAndPrint(true);
}

void serverPF::setGameMain(){
    sati::getInstance()->inputStatementBuffer = "3)Exit\r\n";
    sati::getInstance()->accumulateBuffersAndPrint(true);
}

void serverPF::setLobbyMainTwoOrMorePlayers() {
    sati::getInstance()->inputStatementBuffer = "1)Start Game 2)Exit\r\n";
    sati::getInstance()->accumulateBuffersAndPrint(true);
}
