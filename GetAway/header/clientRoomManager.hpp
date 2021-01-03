
#ifndef GETAWAY_CLIENTROOMMANAGER_HPP
#define GETAWAY_CLIENTROOMMANAGER_HPP


#include <istream>
#include <memory>
#include <clientChatManager.hpp>
#include <clientLobbyManager.hpp>
#include "terminalInputBase.hpp"

class clientRoomManager:terminalInputBase {

    std::string playerName;
    std::map<int, std::string> players;
    int myId;

    std::shared_ptr<clientChatManager> chatManager;
    std::shared_ptr<clientLobbyManager> lobbyManager;
    bool gameStarted = false;
    inputType inputTypeExpected;

public:
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
