//
// Created by hassan on 11/6/20.
//

#ifndef GETAWAY_SATI_HPP
#define GETAWAY_SATI_HPP

#include <mutex>
#include <thread>
#include "boost/asio.hpp"


namespace net = boost::asio;
using namespace net::ip;


//singleton for asynchronous terminal input
class sati {
    net::io_context& io;
    std::string messageBuffer;
    std::string inputStatement;
    std::string roundBuffer;
    std::string userIncomingInput;
    std::mutex m;


    bool handlerAssigned = false;
    void (*handlerInt)(int input);
    void (*handlerString)(std::string input);

    bool messageConstraint;
    int lowerBound, upperBound;

    void accumulateAndPrintString();
    explicit sati(net::io_context& io_);

public:
    static sati& getInstance(net::io_context& io_);
    void operator()();

    void setIntHandlerAndConstraints(void (*handlerInt)(int input), int lowerBound_, int upperBound_);
    void setStringHandlerAndConstraints(void (*handlerString)(std::string input_));


    //TODO
    //RE-EVALUATE THESE DECISIONS
    //Currently, there is no function to clear buffers
    //Any of the following function call means, print something
    void messageBufferAppend(const std::string& message);
    //When inputStatementBuffer changes, roundBuffer is cleared.
    void inputStatementBufferChanged(std::string inputStatementNew);
    void roundBufferAppend(const std::string& turnNew);

};



#endif //GETAWAY_SATI_HPP
