//
// Created by hassan on 11/6/20.
//

#include <iostream>
#include <utility>
#include "sati.hpp"
#include <functional>
#include "deckSuit.hpp"

#if defined(_WIN32) || defined(_WIN64)
#include <conio.h>
#endif

sati& sati::getInstanceFirstTime(net::io_context& io_, std::mutex& mut) {
    static sati s{io_, mut};
    oneInstanceOnly = &s;
    return s;
}

sati* sati::getInstance(){
    return oneInstanceOnly;
}

sati::sati(net::io_context &io_, std::mutex& mut) : io{io_}, m{mut}{

}


void sati::setInputType(inputType nextReceiveInputType) {
    std::lock_guard<std::mutex> lock{m.get()};
    receiveInputType = nextReceiveInputType;
    handlerAssigned = true;
}

void sati::printExitMessage(const std::string& message) {
    std::lock_guard<std::mutex> lockGuard(m);
    std::cout<<message<<std::endl;
}

void sati::setBase(inputRead *base_, appState currentAppState_) {
    currentAppState = currentAppState_;
    std::lock_guard<std::mutex> lockGuard(m);
    base = base_;
}

void sati::operator()() {
#ifdef __linux__
    system("stty raw");
#endif

    while(true){
#if defined(_WIN32) || defined(_WIN64)
        int c = getch();
#endif
#ifdef __linux__
        int c = getchar();
#endif
        if(c == 3) //ctrl + c
        {
#ifdef __linux__
            system("stty cooked");
#endif
            exit(0);
        }else if( c == 127){
            std::lock_guard<std::mutex> lok(m.get());
            if(!userIncomingInput.empty()){
                userIncomingInput.pop_back();
            }
            accumulateBuffersAndPrint(false);
        }else if(c == 10 || c == 13){ //cr pressed
            std::lock_guard<std::mutex> lok(m.get());
            if(handlerAssigned && (base != nullptr)){
                net::post(io, [handler = base, expectedInput = receiveInputType,
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

void sati::accumulateBuffersAndPrint(bool lock) {
    std::string toPrint;
#ifdef SERVERMACRO
    if(currentAppState == appState::HOME){
        toPrint = inputStatementBuffer;
    }
    if(currentAppState == appState::LOBBY) {

    }
    if(currentAppState == appState::GAME){

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
#ifdef __linux__
    system("clear");
#endif
#if defined(_WIN32) || defined(_WIN64)
    system("cls");
#endif
    std::cout<<toPrint;
}

void sati::accumulatePrint(){
    accumulateBuffersAndPrint(true);
}