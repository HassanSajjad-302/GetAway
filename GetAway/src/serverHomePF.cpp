#include "serverHome.hpp"
#include "sati.hpp"

void serverHome::PF::setHomeMain() {
    sati::getInstance()->nonMessageBuffer = "1)Start Server 2)Exit\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}

void serverHome::PF::setHomeChangeServerName(){
    sati::getInstance()->nonMessageBuffer = "Enter Server Name. Press Enter To Use Default.\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}