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

//TODO
//Backspace not implemented yet.
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
            if(!handlerAssigned){
                std::cout<<"Receiving Input. But No Handler Assigned \r"<<std::endl;
                system("stty cooked");
                throw std::logic_error("Receiving Input. But No Handler Assigned \r");
            }
            std::lock_guard<std::mutex> lok(m.get());
            if(messageConstraint){
                handlerAssigned = false;
                net::post(io, [input = std::move(userIncomingInput), handler = base](){
                    handler->inputString(input); //Assumed that this function calls accumulateAndBuffersAndPrint(); //That's requirement
                });
            }else{
                try{
                    int num = std::stoi(userIncomingInput);
                    if(num>=lowerBound && num<=upperBound){
                        handlerAssigned = false;
                        net::post(io, [num, handler = base](){
                            handler->inputInt(num); //Assumed that this function calls accumulateAndBuffersAndPrint(); //That's requirement
                        });
                    }else{
                        userIncomingInput.clear();
                        accumulateBuffersAndPrintWithOutLock();
                        std::cout<<"Please enter integer in range \r"<<std::endl;
                    }
                }
                catch (std::invalid_argument& e) {
                    userIncomingInput.clear();
                    accumulateBuffersAndPrintWithOutLock();
                    std::cout<<"Invalid Input. \r"<<std::endl;
                }
                catch(std::out_of_range& e){
                    userIncomingInput.clear();
                    accumulateBuffersAndPrintWithOutLock();
                    std::cout<<"Invalid Input. \r"<<std::endl;
                }
                userIncomingInput.clear();
            }
        }
        else{
            userIncomingInput += c;
            accumulateBuffersAndPrintWithLock();
        }
    }
}

void sati::setIntHandlerAndConstraints(inputRead* base_, int lowerBound_, int upperBound_) {
    std::lock_guard<std::mutex> lok(m.get());
    if(handlerAssigned){
        throw std::logic_error("constraint already assigned\r");
    }
    base = base_;
    lowerBound = lowerBound_;
    upperBound = upperBound_;
    messageConstraint = false;
    handlerAssigned = true;
}

void sati::setStringHandlerAndConstraints(inputRead* base_) {
    std::lock_guard<std::mutex> lok(m.get());
    if(handlerAssigned){
        throw std::logic_error("constraint already assigned\r");
    }
    base = base_;
    messageConstraint = true;
    handlerAssigned = true;
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
