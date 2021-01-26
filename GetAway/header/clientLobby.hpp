
#ifndef GETAWAY_CLIENTLOBBY_HPP
#define GETAWAY_CLIENTLOBBY_HPP


#include <istream>
#include <memory>
#include "clientChat.hpp"
#include "terminalInputBase.hpp"
#include "asio/io_context.hpp"
#include "session.hpp"
class clientGetAway;
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
    std::shared_ptr<session<clientLobby>> clientRoomSession;
    std::shared_ptr<clientChat> chatManager;
    std::shared_ptr<clientGetAway> lobbyManager;
    bool gameStarted = false;
    void gameFinished();

    explicit clientLobby(asio::io_context& io_);

    void join(std::shared_ptr<session<clientLobby>> clientRoomSession_);

    void packetReceivedFromNetwork(std::istream &in, int receivedPacketSize);

    void SELFANDSTATEReceived();

    void PLAYERJOINEDOrPLAYERLEFTReceived();

    void input(std::string inputString, inputType inputReceivedType) override;

    void setInputType(inputType inputType);

    void exitApplication(bool backToHome);

    void setBaseAndInputTypeFromclientChatMessage();
};


#endif //GETAWAY_CLIENTLOBBY_HPP
