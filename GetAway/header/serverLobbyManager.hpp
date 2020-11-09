#ifndef GETAWAY_SERVERLOBBYMANAGER_HPP
#define GETAWAY_SERVERLOBBYMANAGER_HPP

#include <memory>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <iostream>
#include <map>
#include <tuple>
#include "session.hpp"
#include "serverListener.hpp"
#include "serverGameManager.hpp"
#include "messageTypeEnums.hpp"
class serverLobbyManager
{
    //TODO
    //Should be supplied from somewhere else
    std::chrono::seconds timePerTurn = std::chrono::seconds(60);
    std::map<int, std::tuple<const std::string,
    std::shared_ptr<session<serverLobbyManager, true>>>> gameData;

    friend std::ostream& operator<<(std::ostream& out, serverLobbyManager& state);
    friend std::istream& operator>>(std::istream& in, serverLobbyManager& state);

    std::shared_ptr<serverListener> serverlistener; //This is passed next to lobby which uses it to cancel accepting
    std::shared_ptr<serverGameManager> nextManager;

    std::string chatMessageReceived;
    std::string playerNameAdvanced;
    std::string playerNameFinal;
    lobbyMessageType messageSendingType;
public:
    void setPlayerNameAdvanced(std::string advancedPlayerName_);

public:
    explicit
    serverLobbyManager(std::shared_ptr<serverListener> serverlistener_);
    void uselessWriteFunction(int id);

    //Used-By-Session
    int join  (std::shared_ptr<session<serverLobbyManager, true>> lobbySession);
    int excitedSessionId;
    int receivedPacketSize;
    void leave (int id);



    void sendSelfAndStateToOneAndPlayerJoinedToRemaining();

    void sendChatMessageToAllExceptSenderItself();

    void managementPlayerJoined();

    void managementPlayerLeft();

    void sendPlayerLeftToAllExceptOne();
};
#endif //GETAWAY_SERVERLOBBYMANAGER_HPP
