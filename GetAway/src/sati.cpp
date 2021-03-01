#ifndef ANDROID

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

void sati::setBase(terminalInputBase *base_, appState currentAppState_) {
    currentAppState = currentAppState_;
    std::lock_guard<std::mutex> lockGuard(m.get());
    base = base_;
}

void sati::setBaseAndInputType(terminalInputBase* base_, inputType nextReceiveInputType){
    std::lock_guard<std::mutex> lock{m.get()};
    base = base_;
    receiveInputType = nextReceiveInputType;
    handlerAssigned = true;
}

void sati::setBaseAndCurrentStateAndInputType(terminalInputBase* base_, appState currentAppState_, inputType nextReceivedInputType){
    currentAppState = currentAppState_;
    std::lock_guard<std::mutex> lock{m.get()};
    base = base_;
    receiveInputType = nextReceivedInputType;
    handlerAssigned = true;
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
                        emptybroadcastMessage = std::move(userIncomingInput)](){
                    handler->input(emptybroadcastMessage, expectedInput);
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

//A completely new function and refactorization of sati.cpp
void sati::accumulateBuffersAndPrint(bool lock) {
    resourceStrings::clearAndPrint(messageBuffer, nonMessageBuffer, inputStatement, userIncomingInput, m.get(), lock);
}

#endif