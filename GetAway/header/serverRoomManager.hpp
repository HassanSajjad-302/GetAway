
#ifndef GETAWAY_SERVERROOMMANAGER_HPP
#define GETAWAY_SERVERROOMMANAGER_HPP

#include <map>
#include <memory>
#include "serverListener.hpp"
#include "asio/io_context.hpp"
#include "session.hpp"
#include "terminalInputBase.hpp"
#include "serverLobbyManager.hpp"

class serverChatManager;
class serverRoomManager: terminalInputBase{

    std::map<int, std::tuple<const std::string,
            std::shared_ptr<session<serverRoomManager, true>>>> players;

    std::string playerNameAdvanced;
    std::shared_ptr<serverListener> serverlistener;
    asio::io_context& io;

    std::shared_ptr<serverChatManager> chatManager;
    std::shared_ptr<serverLobbyManager> lobbyManager;

    inputType inputTypeExpected;
public:
    bool gameStarted = false;
    serverRoomManager(std::shared_ptr<serverListener> serverlistener_, asio::io_context& io_);
    void setPlayerNameAdvanced(std::string playerNameAdvacned_);
    void shutDown();

    int join(std::shared_ptr<session<serverRoomManager, true>> roomSession);

    void managementJoin(int excitedSessionId, const std::string &playerNameFinal);

    void uselessWriteFunction(int id);

    void sendPLAYERJOINEDToAllExceptOne(int excitedSessionId);

    void sendPLAYERLEFTToAllExceptOne(int excitedSessionId);

    void leave(int id);

    void goBackToServerListener();

    void setInputType(inputType type);

    void input(std::string inputString, inputType inputReceivedType);

    void packetReceivedFromNetwork(std::istream &in, int receivedPacketSize, int sessionId);
};


#endif //GETAWAY_SERVERROOMMANAGER_HPP
