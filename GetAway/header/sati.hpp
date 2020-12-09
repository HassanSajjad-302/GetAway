//
// Created by hassan on 11/6/20.
//

#ifndef GETAWAY_SATI_HPP
#define GETAWAY_SATI_HPP

#include <set>
#include <mutex>
#include <thread>
#include "boost/asio.hpp"
#include "messageTypeEnums.hpp"
#include "deckSuit.hpp"
#include "appState.hpp"
#include "lobbyPF.hpp"
#include "gamePF.hpp"
#include "messagePF.hpp"
#include "homePF.hpp"

namespace net = boost::asio;
using namespace net::ip;

class inputRead{
public:
    virtual void input(std::string inputString, inputType inputReceivedType) =0;
};

//singleton for asynchronous terminal input
class sati {
private:
    net::io_context& io;
    std::reference_wrapper<std::mutex> m; //This mutex needs to be locked when this class members changes.
    // Or some other thread wants to print on screen.
    appState currentAppState;

    //Buffers For Holding And RePrinting
    std::string userIncomingInput;

    std::string messageBuffer; //messages
    std::string inputStatementBuffer;

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
    bool handlerAssigned = false;
    inputRead* base = nullptr;

    void accumulateBuffersAndPrint(bool lock);

    explicit sati(net::io_context& io_, std::mutex& mut);
    static inline sati* oneInstanceOnly;

    friend class lobbyPF;
    friend class homePF;
    friend class gamePF;
    friend class messagePF;
public:
    static sati& getInstanceFirstTime(net::io_context& io_, std::mutex& mut);
    static sati* getInstance();
    void setInputType(inputType nextReceiveInputType);
    void printExitMessage(const std::string& message);
    void setBase(inputRead* base_, appState currentAppState_);
    void operator()();

    inputType receiveInputType;
    void accumulatePrint();
};

#endif //GETAWAY_SATI_HPP
