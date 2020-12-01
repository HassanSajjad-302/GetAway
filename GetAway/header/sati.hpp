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
    //Buffers For Holding And RePrinting

    //Common In Both Lobby And Game
    std::string messageBuffer; //messages
    std::string userIncomingInput;
    std::string inputStatementBuffer; //input

    //Only For Lobby
    std::string playersInLobby;

    //Only For Game
    std::string turnSequence;
    std::string turns;
    std::string waitingForTurn;
    std::string timeLeft;
    std::string cardsString;


    bool gameStarted = false;
private:

    //This mutex needs to be locked when this class members changes. Or some other thread wants to
    //print on screen.
    std::reference_wrapper<std::mutex> m;


    bool handlerAssigned = false;
    inputRead* base = nullptr;

    void accumulateBuffersAndPrint(bool lock);

    explicit sati(net::io_context& io_, std::mutex& mut);
    static inline sati* oneInstanceOnly;
public:
    static sati& getInstanceFirstTime(net::io_context& io_, std::mutex& mut);
    static sati* getInstance();
    void setInputType(inputType nextReceiveInputType);
    void printExitMessage(const std::string& message);
    void setBase(inputRead* base_);
    void operator()();

    void setReceiveInputTypeAndGameStarted(inputType nextReceiveInputType, bool gameStarted_);
    inputType receiveInputType;
    //If the function name has accumulate, it will also call accumulate in the end.
    //If the function name has game then it will use game strings above if it has
    //lobby it will use lobby strings above.

    void accumulatePrint();

    void setInputStatementMessagePrint();
    void setInputStatementMessageAccumulatePrint();


    void addMessagePrint(const std::string& playerName, const std::string& message);
    void addMessageAccumulatePrint(const std::string& playerName, const std::string& message);


    //USED ONLY IN LOBBY

    //input-statement-functions
    void setInputStatementHomeLobbyPrint();
    void setInputStatementHomeLobbyAccumulatePrint();

    //others
    void addOrRemovePlayerLobbyPrint(const std::map<int, std::string>& gamePlayer_);
    void addOrRemovePlayerLobbyAccumulatePrint(const std::map<int, std::string>& gamePlayer_);




    //USED ONLY IN GAME

    //input-statement-functions
    void setInputStatementHomeTwoInputGamePrint();
    void setInputStatementHomeTwoInputGameAccumulatePrint();

    void setInputStatementHomeThreeInputGamePrint();
    void setInputStatementHomeThreeInputGameAccumulatePrint();

    void setInputStatement3GamePrint(const std::set<int>& cards_, int deckSuitType);
    void setInputStatement3GameAccumulatePrint(const std::set<int>& cards_, int deckSuitType);

    void setInputStatement3GamePrint(const std::map<int, std::set<int>>& cards_);
    void setInputStatement3GameAccumulatePrint(const std::map<int, std::set<int>>& cards_);

    void setTurnSequenceGamePrint(const std::map<int, std::string>& gamePlayer_, const std::vector<int>& turnSequence_);
    void setTurnSequenceGameAccumulatePrint(const std::map<int, std::string>& gamePlayer_, const std::vector<int>& turnSequence_);

    void
    setRoundTurnsGamePrint(const std::vector<std::tuple<int, int>> &roundTurns,
                           const std::map<int, std::string> &gamePlayers);

    void setRoundTurnsGameAccumulatePrint(const std::vector<std::tuple<int, int>> &roundTurns,
                                          const std::map<int, std::string> &gamePlayers);

    void clearTurnGamePrint();
    void clearTurnGameAccumulatePrint();

    void setWaitingForTurnGamePrint(const std::vector<int>& waitingplayersId, const std::map<int, std::string>& gamePlayers);
    void setWaitingForTurnGameAccumulatePrint(const std::vector<int>& waitingPlayersId, const std::map<int, std::string>& gamePlayers);

    void setTimeLeftGamePrint(int seconds);
    void setTimeLeftGameAccumulatePrint(int seconds);

    void setCardsGamePrint(const std::map<int, std::set<int>>& cardsMap);
    void setCardsGameAccumulatePrint(const std::map<int, std::set<int>>& cardsMap);


};

#endif //GETAWAY_SATI_HPP
