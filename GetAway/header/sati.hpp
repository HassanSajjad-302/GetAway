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

class inputRead{
public:
    virtual void inputInt(int input) = 0;
    virtual void inputString(std::string input) = 0;
};

//singleton for asynchronous terminal input
class sati {
    net::io_context& io;
    std::string messageBuffer;
    std::string roundBufferBefore;
    std::string inputStatement;
    std::string roundBufferAfter;
    std::string userIncomingInput;
    std::reference_wrapper<std::mutex> m;


    bool handlerAssigned = false;
    inputRead* base;

    bool messageConstraint;
    int lowerBound, upperBound;

    void accumulateBuffersAndPrint();
    explicit sati(net::io_context& io_, std::mutex& mut);
    static inline sati* oneInstanceOnly;
public:
    static sati& getInstanceFirstTime(net::io_context& io_, std::mutex& mut);
    static sati* getInstance();
    void operator()();

    void setIntHandlerAndConstraints(inputRead* base_, int lowerBound_, int upperBound_);
    void setStringHandlerAndConstraints(inputRead* base_);


    //TODO
    //RE-EVALUATE THESE DECISIONS
    //Currently, there is no function to clear buffers
    //Any of the following function call means, print something
    void messageBufferAppend(const std::string& message);
    void roundBufferBeforeChanged(std::string roundInfo);
    void inputStatementBufferChanged(std::string inputStatementNew, bool clearRoundBuffer);
    void roundBufferAfterAppend(const std::string& roundInfo);

};



#endif //GETAWAY_SATI_HPP
