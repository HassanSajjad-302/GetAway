//
// Created by hassan on 12/7/20.
//

#include "messagePF.hpp"
#include "sati.hpp"
void messagePF::setInputStatement() {
    sati::getInstance()->inputStatementBuffer = "Please Type The Message \r\n";
}

void messagePF::setInputStatementAccumulate() {
    sati::getInstance()->inputStatementBuffer = "Please Type The Message \r\n";
    sati::getInstance()->accumulateBuffersAndPrint(true);
}

void messagePF::add(const std::string &playerName, const std::string &message) {
    sati::getInstance()->messageBuffer += playerName + " : " + message + "\r\n";
}
void messagePF::addAccumulate(const std::string& playerName, const std::string& message) {
    sati::getInstance()->messageBuffer += playerName + " : " + message + "\r\n";
    sati::getInstance()->accumulateBuffersAndPrint(true);
}