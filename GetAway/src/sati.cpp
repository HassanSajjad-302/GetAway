//
// Created by hassan on 11/6/20.
//

#include <iostream>
#include <utility>
#include "sati.hpp"
#include <functional>

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

void sati::operator()() {
    system("stty raw");
    while(true){
        int c = getchar();
        if(c == 3) //ctrl + c
        {
            system("stty cooked");
            exit(0);
        }else if( c == 127){
            std::lock_guard<std::mutex> lok(m.get());
            if(!userIncomingInput.empty()){
                userIncomingInput.pop_back();
            }
            accumulateBuffersAndPrintWithOutLock();
        }else if(c == 10 || c == 13){ //cr pressed
            //TODO
            //For The Current There is no moment when programme should not be accepting input
            std::lock_guard<std::mutex> lok(m.get());
            if(!handlerAssigned){
                std::cout<<"Receiving Input. But No Handler Assigned \r"<<std::endl;
                system("stty cooked");
                throw std::logic_error("Receiving Input. But No Handler Assigned \r");
            }
            net::post(io, [handler = base, expectedInput = receiveInputType,
                           str = std::move(userIncomingInput)](){
                handler->input(str, expectedInput);
            });
            handlerAssigned = false;
            userIncomingInput.clear();
        }
        else{
            userIncomingInput += c;
            accumulateBuffersAndPrintWithLock();
        }
    }
}

void sati::messageBufferAppend(const std::string& message) {
    messageBuffer += message;
    accumulateBuffersAndPrintWithLock();
}

void sati::roundBufferBeforeChanged(std::string roundInfo) {
    roundBufferBefore.clear();
    roundBufferBefore = std::move(roundInfo);
    accumulateBuffersAndPrintWithLock();
}

void sati::inputStatementBufferChanged(std::string inputStatementNew, bool clearRoundBuffer) {
    inputStatement = std::move(inputStatementNew);
    if(clearRoundBuffer){
        roundBufferBefore.clear();
        roundBufferAfter.clear();
    }
    accumulateBuffersAndPrintWithLock();
}

void sati::roundBufferAfterAppend(const std::string& roundInfo) {
    roundBufferAfter += roundInfo;
    accumulateBuffersAndPrintWithLock();
}

void sati::accumulateBuffersAndPrintWithLock() {
    std::string toPrint = messageBuffer + roundBufferBefore + inputStatement + roundBufferAfter;
    {
        std::lock_guard lock(m.get());
        toPrint += userIncomingInput;
    }
    system("clear");
    std::cout<<toPrint;
}

void sati::accumulateBuffersAndPrintWithOutLock() {
    std::string toPrint = messageBuffer + roundBufferBefore + inputStatement + roundBufferAfter;
    toPrint += userIncomingInput;
    system("clear");
    std::cout<<toPrint;
}

void sati::printExitMessage(std::string message) {
    std::lock_guard<std::mutex> lockGuard(m);
    std::cout<<message<<std::endl;
}

void sati::setInputType(inputType nextReceiveInputType) {
    receiveInputType = nextReceiveInputType;
    std::lock_guard<std::mutex> lock{m.get()};
    handlerAssigned = true;
}

void sati::setBase(inputRead *base_) {
    base = base_;
}
