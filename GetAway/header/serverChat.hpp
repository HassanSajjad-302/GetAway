
#ifndef GETAWAY_SERVERCHAT_HPP
#define GETAWAY_SERVERCHAT_HPP

#include <map>
#include <string>
#include <memory>
#include "serverSession.hpp"

class serverLobby;
class serverChat {

    const std::map<int, std::tuple<std::string,
            std::unique_ptr<serverSession<serverLobby>>>>& players;
public:
    explicit serverChat(const std::map<int, std::tuple<std::string,
            std::unique_ptr<serverSession<serverLobby>>>>& players_);

    void packetReceivedFromNetwork(std::istream &in, int receivedPacketSize, int sessionId);

    void CHATMESSAGEReceived(const std::string &chatMessageReceived, int excitedSessionId);
};


#endif //GETAWAY_SERVERCHAT_HPP
