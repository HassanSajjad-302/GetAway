#ifdef ANDROID

#include <utility>
#include "satiAndroid.hpp"
#include "deckSuit.hpp"
#include "asio/post.hpp"
#include "resourceStrings.hpp"
#if defined(_WIN32) || defined(_WIN64)
#include <conio.h>
#endif

sati& sati::getInstanceFirstTime(asio::io_context& io_) {
    static sati s{io_};
    oneInstanceOnly = &s;
    return s;
}

sati* sati::getInstance(){
    return oneInstanceOnly;
}

sati::sati(asio::io_context &io_) : io{io_}{

}

void sati::printExitMessage(const std::string& message) {
    resourceStrings::print(message + "\r\n");
}

void sati::setBase(terminalInputBase *base_, appState currentAppState_) {
    currentAppState = currentAppState_;
    base = base_;
}

void sati::setBaseAndInputType(terminalInputBase* base_, inputType nextReceiveInputType){
    base = base_;
    receiveInputType = nextReceiveInputType;
    handlerAssigned = true;
}

void sati::setBaseAndCurrentStateAndInputType(terminalInputBase* base_, appState currentAppState_, inputType nextReceivedInputType){
    base = base_;
    currentAppState = currentAppState_;
    receiveInputType = nextReceivedInputType;
    handlerAssigned = true;
}

void sati::operator()(std::string userIncomingInput) {
    asio::post(io, [handler = base, expectedInput = receiveInputType,
            emptybroadcastMessage = std::move(userIncomingInput)](){
        handler->input(emptybroadcastMessage, expectedInput);
    });
    handlerAssigned = false;
}

void sati::accumulatePrint(){
    accumulateBuffersAndPrint();
}

void sati::accumulateBuffersAndPrint() {
    std::string toPrint;
    toPrint = messageBuffer + nonMessageBuffer + inputStatement + userIncomingInput;
    resourceStrings::clearAndPrint(toPrint);
}

#endif