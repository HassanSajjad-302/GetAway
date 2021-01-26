
#ifndef GETAWAY_SERVERCHAT_HPP
#define GETAWAY_SERVERCHAT_HPP

#include <map>
#include <string>
#include <memory>
#include "serverLobby.hpp"

class serverLobby;
class serverChat {

    const std::map<int, std::tuple<const std::string,
            std::shared_ptr<session<serverLobby, true>>>>& players;
public:
    serverChat(const std::map<int, std::tuple<const std::string,
            std::shared_ptr<session<serverLobby, true>>>>& players_);

    void packetReceivedFromNetwork(std::istream &in, int receivedPacketSize, int sessionId);

    void CHATMESSAGEReceived(const std::string &chatMessageReceived, int excitedSessionId);
};


#endif //GETAWAY_SERVERCHAT_HPP
