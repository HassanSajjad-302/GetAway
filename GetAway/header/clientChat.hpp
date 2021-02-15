
#ifndef GETAWAY_CLIENTCHAT_HPP
#define GETAWAY_CLIENTCHAT_HPP

#include "terminalInputBase.hpp"
#include <istream>
#include <map>

class clientLobby;
class clientChat: public terminalInputBase {
    class PF {
    public:
        static void setInputStatementAccumulate();
        static void addAccumulate(const std::string& playerName, const std::string& message);

    };
    clientLobby& lobbyManager;
    int myId;
    const std::string& playerName;
    const std::map<int, std::string>& players;
    std::string chatMessageString;
    int chatMessageInt;

public:
    clientChat(clientLobby& lobbyManager_, const std::map<int, std::string> &players_, const std::string &playerName_, int myId_);

    void packetReceivedFromNetwork(std::istream &in, int receivedPacketSize);

    void input(std::string inputString, inputType receivedInputType_) override;

    void sendCHATMESSAGE();

    void sendCHATMESSAGEHandler();

    void setBaseAndInputTypeForMESSAGESTRING();
};


#endif //GETAWAY_CLIENTCHAT_HPP
