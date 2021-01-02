
#ifndef GETAWAY_SERVERCHATMANAGER_HPP
#define GETAWAY_SERVERCHATMANAGER_HPP

#include <map>
#include <string>
#include <memory>
#include "serverRoomManager.hpp"

class serverRoomManager;
class serverChatManager {

    const std::map<int, std::tuple<const std::string,
            std::shared_ptr<session<serverRoomManager, true>>>>& players;
public:
    serverChatManager(const std::map<int, std::tuple<const std::string,
            std::shared_ptr<session<serverRoomManager, true>>>>& players_);

    void packetReceivedFromNetwork(std::istream &in, int receivedPacketSize, int sessionId);

    void CHATMESSAGEReceived(const std::string &chatMessageReceived, int excitedSessionId);
};


#endif //GETAWAY_SERVERCHATMANAGER_HPP
