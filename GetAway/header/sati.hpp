//
// Created by hassan on 11/6/20.
//

#ifndef GETAWAY_SATI_HPP
#define GETAWAY_SATI_HPP

#include <mutex>
#include <thread>
#include "boost/asio.hpp"
#include "messageTypeEnums.hpp"


namespace net = boost::asio;
using namespace net::ip;

class inputRead{
public:
    virtual void input(std::string inputString, inputType inputReceivedType) =0;
};

//singleton for asynchronous terminal input
class sati {
    net::io_context& io;
    std::string messageBuffer;
    std::string roundBufferBefore;
    std::string inputStatement;
    std::string roundBufferAfter;
    std::string userIncomingInput;
    //This mutex needs to be locked when this class members changes. Or some other thread wants to
    //print on screen.
    std::reference_wrapper<std::mutex> m;


    bool handlerAssigned = false;
    inputRead* base;


    void accumulateBuffersAndPrintWithOutLock();
    explicit sati(net::io_context& io_, std::mutex& mut);
    static inline sati* oneInstanceOnly;
public:
    static sati& getInstanceFirstTime(net::io_context& io_, std::mutex& mut);
    static sati* getInstance();
    void operator()();

    //void setIntHandlerAndConstraints(inputRead* base_, int lowerBound_, int upperBound_);
    //void setStringHandlerAndConstraints(inputRead* base_);

    inputType receiveInputType;
    void setInputType(inputType nextReceiveInputType);

    //TODO
    //RE-EVALUATE THESE DECISIONS
    //Currently, there is no function to clear buffers
    //Any of the following function call means, print something
    void messageBufferAppend(const std::string& message); //messages
    void roundBufferBeforeChanged(std::string roundInfo); //Players in lobby are, Your cards
    void inputStatementBufferChanged(std::string inputStatementNew, bool clearRoundBuffer); //input statement
    void roundBufferAfterAppend(const std::string& roundInfo); //turns played by other players

    void printExitMessage(std::string message);
    void accumulateBuffersAndPrintWithLock();
    void setBase(inputRead* base_);
};



#endif //GETAWAY_SATI_HPP
