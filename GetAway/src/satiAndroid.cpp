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


void sati::setInputType(inputType nextReceiveInputType) {
    receiveInputType = nextReceiveInputType;
    handlerAssigned = true;
}

void sati::printExitMessage(const std::string& message) {
    resourceStrings::print(message + "\r\n");
}

void sati::setBase(inputRead *base_, appState currentAppState_) {
    currentAppState = currentAppState_;
    base = base_;
}

void sati::operator()(std::string userIncomingInput) {
    asio::post(io, [handler = base, expectedInput = receiveInputType,
            str = std::move(userIncomingInput)](){
        handler->input(str, expectedInput);
    });
    handlerAssigned = false;
}

void sati::accumulatePrint(){
    accumulateBuffersAndPrint();
}

void sati::accumulateBuffersAndPrint() {
    std::string toPrint;
#ifdef SERVERMACRO
    if(currentAppState == appState::HOME){
        toPrint = inputStatementBuffer;
    }
    if(currentAppState == appState::LOBBY) {
        toPrint = inputStatementBuffer;
    }
    if(currentAppState == appState::GAME){
        toPrint = inputStatementBuffer;
    }
#endif
#ifdef CLIENTMACRO
    if(currentAppState == appState::HOME){
        toPrint += inputStatementBuffer + errorMessage;
    }
    if(currentAppState == appState::LOBBY) {
        if(!messageBuffer.empty()){
            toPrint = messageBuffer + "\r\n";
        }
        toPrint += playersInLobby  + "\r\n" + inputStatementBuffer + "\r\n";
    }
    if(currentAppState == appState::GAME){
        if(!messageBuffer.empty()){
            toPrint = messageBuffer + "\r\n";
        }
        toPrint += turnSequence  + "\r\n";
        toPrint += turns + "\r\n" + waitingForTurn + "\r\n" + cardsString + "\r\n" + inputStatementBuffer + "\r\n";
    }
#endif
    resourceStrings::clearAndPrint(toPrint);
}
