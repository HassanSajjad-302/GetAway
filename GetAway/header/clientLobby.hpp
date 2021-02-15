
#ifndef GETAWAY_CLIENTLOBBY_HPP
#define GETAWAY_CLIENTLOBBY_HPP


#include <istream>
#include <memory>
#include "clientChat.hpp"
#include "terminalInputBase.hpp"
#include "asio/io_context.hpp"
#include "clientSession.hpp"
#include "clientGetAway.hpp"
class clientLobby: public terminalInputBase {

    class PF {
    public:
        static inline std::string playersInLobby;
        static inline std::string inputStatement = "1)Send Message 2)Leave 3)Exit\r\n";
        //others
        static void addOrRemovePlayerAccumulate(const std::map<int, std::string>& gamePlayer_);

        static void setInputStatementHomeAccumulate();
    };

    asio::io_context& io;
    std::string playerName;
    std::map<int, std::string> players;

    inputType inputTypeExpected;

public:
    int myId;
    clientSession<clientLobby, false, asio::io_context&, std::string>& clientLobbySession;
    std::unique_ptr<clientChat> clientChatPtr;
    std::unique_ptr<clientGetAway> clientGetAwayPtr;
    bool gameStarted = false;
    void gameFinished();

    explicit clientLobby(clientSession<clientLobby, false, asio::io_context&, std::string>& clientLobbySession_,
                         asio::io_context& io_, std::string playerName);

    void packetReceivedFromNetwork(std::istream &in, int receivedPacketSize);

    void SELFANDSTATEReceived();

    void PLAYERJOINEDOrPLAYERLEFTReceived();

    void input(std::string inputString, inputType inputReceivedType) override;

    void setInputType(inputType inputType);

    void exitApplication(bool backToHome);

    void setBaseAndInputTypeFromclientChatMessage();
};


#endif //GETAWAY_CLIENTLOBBY_HPP
