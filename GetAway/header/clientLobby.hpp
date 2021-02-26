
#ifndef GETAWAY_CLIENTLOBBY_HPP
#define GETAWAY_CLIENTLOBBY_HPP


#include <istream>
#include <memory>
#include "clientChat.hpp"
#include "terminalInputBase.hpp"
#include "asio/io_context.hpp"
#include "clientSession.hpp"
#include "clientGetAway.hpp"
#include "clientBluff.hpp"
#include "serverListener.hpp"

class clientLobby: public terminalInputBase {

    class PF {
    public:
        static inline std::string playersInLobby;
        static inline std::string inputStatement = "3)Leave 4)Exit\r\n";
        //others
        static void addOrRemovePlayerAccumulate(const std::map<int, std::string>& gamePlayer_);

        static void setInputStatementHomeAccumulate();
    };

    asio::io_context& io;
    std::string myName;
    std::map<int, std::string> players;

    bool clientOnly = true;
    serverListener* listener;
    constants::gamesEnum gameSelected;
public:
    int myId;
    clientSession<clientLobby, asio::io_context&, std::string, serverListener*, bool, constants::gamesEnum>& clientLobbySession;
    std::unique_ptr<clientChat> clientChatPtr;
    //todo
    //make these pointers private
    std::unique_ptr<clientGetAway> clientGetAwayPtr;
    std::unique_ptr<Bluff::clientBluff> clientBluffPtr;
    bool gameStarted = false;
    void gameFinished();

    explicit clientLobby(clientSession<clientLobby, asio::io_context&, std::string, serverListener*, bool,
                         constants::gamesEnum>& clientLobbySession_, asio::io_context& io_, std::string playerName,
                         serverListener* listener_, bool isItClientOnly_, constants::gamesEnum gameSelected_);

    void packetReceivedFromNetwork(std::istream &in, int receivedPacketSize);

    void input(std::string inputString, inputType inputReceivedType) override;

    void setInputType(inputType inputType);

    void exitApplication(bool backToHome);

    void setBaseAndInputTypeFromclientChatMessage();

    void run();
};


#endif //GETAWAY_CLIENTLOBBY_HPP
