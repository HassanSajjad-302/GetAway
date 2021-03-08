#ifndef GETAWAY_SATI_HPP
#define GETAWAY_SATI_HPP

#include <set>
#include <thread>
#include "messageTypeEnums.hpp"
#include "deckSuit.hpp"
#include "appState.hpp"

#include "asio/io_context.hpp"
#include "inputType.h"
#include "terminalInputBase.hpp"

//singleton for asynchronous terminal input
class sati {
private:
    asio::io_context& io;
    appState currentAppState;
    //Buffers For Holding And RePrinting
    std::string userIncomingInput;
    void accumulateBuffersAndPrint();

public:
    std::string messageBuffer; //messages
    std::string nonMessageBuffer;
    std::string inputStatement;

    bool handlerAssigned = false;
    terminalInputBase* base = nullptr;


    explicit sati(asio::io_context& io_);
    static inline sati* oneInstanceOnly;

    static sati& getInstanceFirstTime(asio::io_context& io_);
    static sati* getInstance();
    void setInputType(inputType nextReceiveInputType);
    void printExitMessage(const std::string& message);
    void setBase(terminalInputBase* base_, appState currentAppState_);
    void setBaseAndInputType(terminalInputBase *base_, inputType nextReceiveInputType);
    void setBaseAndCurrentStateAndInputType(terminalInputBase *base_, appState currentAppState_,
                                            inputType nextReceivedInputType);
    void operator()(std::string userIncomingInput);

    inputType receiveInputType;
    void accumulatePrint();

};

#endif //GETAWAY_SATI_HPP
