#ifdef ANDROID
#include "satiAndroid.hpp"
#else
#ifndef GETAWAY_SATI_HPP
#define GETAWAY_SATI_HPP

#include <set>
#include <mutex>
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
    std::reference_wrapper<std::mutex> m; //This mutex needs to be locked when this class members changes.
    // Or some other thread wants to clearAndPrint on screen.
    appState currentAppState;
    //Buffers For Holding And RePrinting
    std::string userIncomingInput;
    void accumulateBuffersAndPrint(bool lock);
    void accumulateBuffersAndPrint();

public:
    std::string messageBuffer; //messages
    std::string nonMessageBuffer;
    std::string inputStatement;

    bool handlerAssigned = false;
    terminalInputBase* base = nullptr;


    explicit sati(asio::io_context& io_, std::mutex& mut);
    static inline sati* oneInstanceOnly;

    static sati& getInstanceFirstTime(asio::io_context& io_, std::mutex& mut);
    static sati* getInstance();
    void setInputType(inputType nextReceiveInputType);
    void printExitMessage(const std::string& message);
    void setBase(terminalInputBase* base_, appState currentAppState_);
    void setBaseAndInputType(terminalInputBase *base_, inputType nextReceiveInputType);
    void setBaseAndCurrentStateAndInputType(terminalInputBase *base_, appState currentAppState_,
                                            inputType nextReceivedInputType);
    void operator()();

    inputType receiveInputType;
    void accumulatePrint();

};

#endif //GETAWAY_SATI_HPP
#endif
