
#ifndef GETAWAY_CLIENTCHATMANAGER_HPP
#define GETAWAY_CLIENTCHATMANAGER_HPP


#include <istream>
#include <map>

class clientChatManager {

    int myId;
    const std::string& playerName;
    const std::map<int, std::string>& players;

public:
    clientChatManager(const std::map<int, std::string> &players_, const std::string &playerName_, int id);

    void packetReceivedFromNetwork(std::istream &in, int receivedPacketSize);

    void input(const std::string &inputString);

    void sendCHATMESSAGE();
};


#endif //GETAWAY_CLIENTCHATMANAGER_HPP
