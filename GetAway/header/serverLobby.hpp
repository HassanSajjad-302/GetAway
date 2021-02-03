
#ifndef GETAWAY_SERVERLOBBY_HPP
#define GETAWAY_SERVERLOBBY_HPP

#include <map>
#include <memory>
#include "serverListener.hpp"
#include "asio/io_context.hpp"
#include "session.hpp"
#include "terminalInputBase.hpp"
#include "serverGetAway.hpp"

class serverChat;
class serverLobby: terminalInputBase{
    class PF {
    public:
        static void setLobbyMainTwoOrMorePlayers();
        static void setGameMain();

    };
    std::map<int, std::tuple<const std::string,
            std::shared_ptr<session<serverLobby, true>>>> players;

    std::string playerNameAdvanced;
    std::shared_ptr<serverListener> serverlistener;
    asio::io_context& io;

    std::shared_ptr<serverChat> chatManager;
    std::shared_ptr<serverGetAway> lobbyManager;

    inputType inputTypeExpected;
public:
    bool gameStarted = false;
    serverLobby(std::shared_ptr<serverListener> serverlistener_, asio::io_context& io_);
    void setPlayerNameAdvanced(std::string playerNameAdvacned_);
    void shutDown();

    int join(std::shared_ptr<session<serverLobby, true>> roomSession);

    void managementJoin(int excitedSessionId, const std::string &playerNameFinal);

    void sendPLAYERJOINEDToAllExceptOne(int excitedSessionId);

    void sendPLAYERLEFTToAllExceptOne(int excitedSessionId);

    void leave(int id);

    void setInputType(inputType type);

    void input(std::string inputString, inputType inputReceivedType) override;

    void packetReceivedFromNetwork(std::istream &in, int receivedPacketSize, int sessionId);

    void gameExitFinished();

    void sendPLAYERLEFTDURINGGAMEToAllExceptOne(int id);
};


#endif //GETAWAY_SERVERLOBBY_HPP
