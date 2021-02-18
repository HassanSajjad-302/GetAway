#ifndef GETAWAY_SATI_HPP
#define GETAWAY_SATI_HPP

#include <set>
#include <thread>
#include "messageTypeEnums.hpp"
#include "deckSuit.hpp"
#include "appState.hpp"
#include "clientGetAwayPF.hpp"
#include "clientGetAwayPF.hpp"
#include "PF.hpp"
#include "homePF.hpp"
#include "asio/io_context.hpp"
#include "inputType.h"
#include "terminalInputBase.hpp"

//singleton for asynchronous terminal input
class sati {
private:
    asio::io_context& io;
    appState currentAppState;
public:
    //Buffers For Holding And RePrinting
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

    explicit sati(asio::io_context& io_);
    static inline sati* oneInstanceOnly;

    friend class lobbyPF;
    friend class homePF;
    friend class gamePF;
    friend class messagePF;

    static sati& getInstanceFirstTime(asio::io_context& io_);
    static sati* getInstance();
    void setInputType(inputType nextReceiveInputType);
    void printExitMessage(const std::string& message);
    void setBase(terminalInputBase* base_, appState currentAppState_);
    void operator()(std::string userIncomingInput);

    inputType receiveInputType;
    void accumulatePrint();
};

#endif //GETAWAY_SATI_HPP
