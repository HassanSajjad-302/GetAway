//
// Created by hassan on 11/6/20.
//

#include <iostream>
#include <utility>
#include "sati.hpp"


sati& sati::getInstance(net::io_context& io_) {
    static sati s{io_};
    return s;
}


sati::sati(net::io_context &io_) : io{io_}{

}



//TODO
//Backspace not implemented yet.
void sati::operator()() {
    while(true){
        char c = getchar();
        if(c == '\n'){
            std::lock_guard<std::mutex> lok(m);
            //TODO
            //For The Current There is no moment when programme should not be accepting input
            if(!handlerAssigned){
                std::cout<<"Receiving Input. But No Handler Assigned "<<std::endl;
                exit(-1);
            }
            handlerAssigned = false;
            if(messageConstraint){
                net::post(io, [input = std::move(userIncomingInput), handler = handlerString](){
                    handler(input);
                });
            }else{
                try{
                    int num = std::stoi(userIncomingInput);
                    if(num>=lowerBound && num<=upperBound){
                        net::post(io, [num, handler = handlerInt](){
                            handler(num);
                        });
                    }else{
                        std::cout<<"Please enter integer in range"<<std::endl;
                    }
                }
                catch (std::invalid_argument& e) {
                    std::cout<<"Invalid Input."<<std::endl;
                }
                catch(std::out_of_range& e){
                    std::cout<<"Invalid Input."<<std::endl;
                }
                userIncomingInput.clear();
            }
            break;
        }else{
            userIncomingInput += c;
        }
    }
}

void sati::setIntHandlerAndConstraints(void (*handlerInt_)(int), int lowerBound_, int upperBound_) {
    std::lock_guard<std::mutex> lok(m);
    if(handlerAssigned){
        throw std::logic_error("constraint already assigned");
    }
    handlerInt = handlerInt_;
    lowerBound = lowerBound_;
    upperBound = upperBound_;
    handlerAssigned = true;
}

void sati::setStringHandlerAndConstraints(void (*handlerString_)(std::string)) {
    std::lock_guard<std::mutex> lok(m);
    if(handlerAssigned){
        throw std::logic_error("constraint already assigned");
    }
    handlerString = handlerString_;
    handlerAssigned = true;
}

void sati::messageBufferAppend(const std::string& message) {
    messageBuffer += message;
    accumulateAndPrintString();
}

void sati::inputStatementBufferChanged(std::string inputStatementNew) {
    inputStatement = std::move(inputStatementNew);
    roundBuffer.clear();
    accumulateAndPrintString();
}

void sati::roundBufferAppend(const std::string& turnNew) {
    roundBuffer += turnNew;
    accumulateAndPrintString();
}

void sati::accumulateAndPrintString() {
    std::string toPrint = messageBuffer + inputStatement + roundBuffer;
    {
        std::lock_guard lock(m);
        toPrint += userIncomingInput;
    }
    std::cout<<toPrint;
}

