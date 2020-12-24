//
// Created by hassan on 12/9/20.
//

#include "serverPF.hpp"
#ifdef ANDROID
#include "satiAndroid.hpp"
#else
#include "sati.hpp"
#endif

void serverPF::setHomeMain() {
    sati::getInstance()->inputStatementBuffer = "1)Start Server 3)Exit\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}

void serverPF::setHomeChangeServerName(){
    sati::getInstance()->inputStatementBuffer = "Enter Server Number\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}

void serverPF::setLobbyMainOnePlayer() {
    sati::getInstance()->inputStatementBuffer = "1)Exit\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}

void serverPF::setGameMain(){
    sati::getInstance()->inputStatementBuffer = "3)Exit\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}

void serverPF::setLobbyMainTwoOrMorePlayers() {
    sati::getInstance()->inputStatementBuffer = "1)Start Game 2)Exit\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}
