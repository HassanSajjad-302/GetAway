
#include "clientChat.hpp"
#include "sati.hpp"

void clientChat::PF::setInputStatementAccumulate() {
    sati::getInstance()->nonMessageBuffer = "Please Type The Message \r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}

void clientChat::PF::addAccumulate(const std::string& playerName, const std::string& message) {
    sati::getInstance()->messageBuffer += playerName + " : " + message + "\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}