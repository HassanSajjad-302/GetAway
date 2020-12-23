//
// Created by hassan on 11/6/20.
//

#include <utility>
#include "sati.hpp"
#include "deckSuit.hpp"
#include "asio/post.hpp"
#include "resourceStrings.hpp"
#if defined(_WIN32) || defined(_WIN64)
#include <conio.h>
#endif

sati& sati::getInstanceFirstTime(asio::io_context& io_, std::mutex& mut) {
    static sati s{io_, mut};
    oneInstanceOnly = &s;
    return s;
}

sati* sati::getInstance(){
    return oneInstanceOnly;
}

sati::sati(asio::io_context &io_, std::mutex& mut) : io{io_}, m{mut}{

}


void sati::setInputType(inputType nextReceiveInputType) {
    std::lock_guard<std::mutex> lock{m.get()};
    receiveInputType = nextReceiveInputType;
    handlerAssigned = true;
}

void sati::printExitMessage(const std::string& message) {
    std::lock_guard<std::mutex> lockGuard(m);
    resourceStrings::print(message + "\r\n");
}

void sati::setBase(inputRead *base_, appState currentAppState_) {
    currentAppState = currentAppState_;
    std::lock_guard<std::mutex> lockGuard(m);
    base = base_;
}

//external
#ifdef __linux__

void sati::operator()() {
    system("stty raw");
    while(true){
        int c = getchar();
        if(c == 3) //ctrl + c
        {
            system("stty cooked");
            exit(0);
        }
            else if( c == 127){
            std::lock_guard<std::mutex> lok(m.get());
            if(!userIncomingInput.empty()){
                userIncomingInput.pop_back();
            }
            accumulateBuffersAndPrint(false);
        }
        else if(c == 10 || c == 13){ //cr pressed
            std::lock_guard<std::mutex> lok(m.get());
            if(handlerAssigned && (base != nullptr)){
                asio::post(io, [handler = base, expectedInput = receiveInputType,
                        str = std::move(userIncomingInput)](){
                    handler->input(str, expectedInput);
                });
                handlerAssigned = false;
                userIncomingInput.clear();
            }
        }
        else{
            userIncomingInput += c;
            accumulateBuffersAndPrint(true);
        }
    }
}
#endif

//External
#if defined(_WIN32) || defined(_WIN64)

void sati::operator()() {
    while(true){
        int c = _getch();
        if(c == 3) //ctrl + c
        {
            exit(0);
        }
        else if( c == 8){
            std::lock_guard<std::mutex> lok(m.get());
            if(!userIncomingInput.empty()){
                userIncomingInput.pop_back();
            }
            accumulateBuffersAndPrint(false);
        }
        else if(c == 10 || c == 13){ //cr pressed
            std::lock_guard<std::mutex> lok(m.get());
            if(handlerAssigned && (base != nullptr)){
                asio::post(io, [handler = base, expectedInput = receiveInputType,
                        str = std::move(userIncomingInput)](){
                    handler->input(str, expectedInput);
                });
                handlerAssigned = false;
                userIncomingInput.clear();
            }
        }
        else{
            userIncomingInput += c;
            accumulateBuffersAndPrint(true);
        }
    }
}
#endif

void sati::accumulateBuffersAndPrint() {
    accumulateBuffersAndPrint(true);
}

void sati::accumulatePrint(){
    accumulateBuffersAndPrint(true);
}

void sati::accumulateBuffersAndPrint(bool lock) {
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
    if(lock){
        std::lock_guard lockGuard(m.get());
        toPrint += userIncomingInput;
    }
    else{
        toPrint += userIncomingInput;
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
    if(lock){
        std::lock_guard lockGuard(m.get());
        toPrint += userIncomingInput;
    }
    else{
        toPrint += userIncomingInput;
    }
#endif

    resourceStrings::clearAndPrint(toPrint);
}
