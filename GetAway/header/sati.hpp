#ifndef ANDROID
#ifndef GETAWAY_SATI_HPP
#define GETAWAY_SATI_HPP

#include <set>
#include <mutex>
#include <thread>
#include "messageTypeEnums.hpp"
#include "deckSuit.hpp"
#include "appState.hpp"
#include "lobbyPF.hpp"
#include "gamePF.hpp"
#include "messagePF.hpp"
#include "clientHomePF.hpp"
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
public:
    std::string inputStatementBuffer;

#ifdef CLIENTMACRO
    std::string messageBuffer; //messages

    //Only For Lobby
    std::string playersInLobby;

    //Only For Game
    std::string turnSequence;
    std::string turns;
    std::string waitingForTurn;
    std::string timeLeft;
    std::string cardsString;

    //Only For Home
    std::string errorMessage;
#endif
#ifdef SERVERMACRO
    std::string playerJoined;
#endif

    bool handlerAssigned = false;
    terminalInputBase* base = nullptr;

    void accumulateBuffersAndPrint();

    explicit sati(asio::io_context& io_, std::mutex& mut);
    static inline sati* oneInstanceOnly;

    friend class lobbyPF;
    friend class homePF;
    friend class gamePF;
    friend class messagePF;

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
#else
#include "satiAndroid.hpp"
#endif
