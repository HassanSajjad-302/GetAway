
#ifndef GETAWAY_CLIENTROOMMANAGER_HPP
#define GETAWAY_CLIENTROOMMANAGER_HPP


#include <istream>
#include <memory>
#include <clientChatManager.hpp>
#include <clientLobbyManager.hpp>
#include "terminalInputBase.hpp"
#include "asio/io_context.hpp"
class clientRoomManager: public terminalInputBase {

    asio::io_context& io;
    std::string playerName;
    std::map<int, std::string> players;

    inputType inputTypeExpected;
    std::vector<mtr> messageTypeExpected;

public:
    int myId;
    std::shared_ptr<session<clientRoomManager>> clientRoomSession;
    std::shared_ptr<clientChatManager> chatManager;
    std::shared_ptr<clientLobbyManager> lobbyManager;
    bool gameStarted = false;

    clientRoomManager(asio::io_context& io_);

    void join(std::shared_ptr<session<clientRoomManager>> clientRoomSession_);

    void packetReceivedFromNetwork(std::istream &in, int receivedPacketSize);

    void SELFANDSTATEReceived();

    void PLAYERJOINEDOrPLAYERLEFTReceived();

    void input(std::string inputString, inputType inputReceivedType);

    void setInputType(inputType inputType);

    void exitApplication();

    void leaveGame();

    void exitApplicationAmidGame();
};


#endif //GETAWAY_CLIENTROOMMANAGER_HPP
