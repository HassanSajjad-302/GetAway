
#ifndef GETAWAY_CLIENTCHATMANAGER_HPP
#define GETAWAY_CLIENTCHATMANAGER_HPP

#include "terminalInputBase.hpp"
#include "clientRoomManager.hpp"
#include <istream>
#include <map>

class clientChatManager: public terminalInputBase {

    clientRoomManager& roomManager;
    int myId;
    const std::string& playerName;
    const std::map<int, std::string>& players;
    std::string chatMessageString;
    int chatMessageInt;

public:
    clientChatManager(clientRoomManager& roomManager_, const std::map<int, std::string> &players_, const std::string &playerName_, int id);

    void packetReceivedFromNetwork(std::istream &in, int receivedPacketSize);

    void input(const std::string &inputString);

    void sendCHATMESSAGE();
};


#endif //GETAWAY_CLIENTCHATMANAGER_HPP
