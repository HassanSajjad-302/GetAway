
#ifndef GETAWAY_CLIENTCHATMANAGER_HPP
#define GETAWAY_CLIENTCHATMANAGER_HPP

#include "terminalInputBase.hpp"
#include <istream>
#include <map>

class clientRoomManager;
class clientChatManager: public terminalInputBase {

    clientRoomManager& roomManager;
    int myId;
    const std::string& playerName;
    const std::map<int, std::string>& players;
    std::string chatMessageString;
    int chatMessageInt;

public:
    clientChatManager(clientRoomManager& roomManager_, const std::map<int, std::string> &players_, const std::string &playerName_, int myId_);

    void packetReceivedFromNetwork(std::istream &in, int receivedPacketSize);

    void input(std::string inputString, inputType receivedInputType_) override;

    void sendCHATMESSAGE();

    void sendCHATMESSAGEHandler();
};


#endif //GETAWAY_CLIENTCHATMANAGER_HPP
