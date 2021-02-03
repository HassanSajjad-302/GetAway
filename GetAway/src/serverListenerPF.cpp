
#include "serverListener.hpp"
#include "sati.hpp"

void serverListener::PF::setLobbyMainOnePlayer() {
    sati::getInstance()->nonMessageBuffer = "2)Close Server 3)Exit\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}