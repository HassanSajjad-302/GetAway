
#ifndef GETAWAY_SERVERLOBBY_HPP
#define GETAWAY_SERVERLOBBY_HPP

#include <map>
#include <memory>
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "session.hpp"
#include "terminalInputBase.hpp"
#include "serverGetAway.hpp"
#include "serverChat.hpp"
class serverListener;
class serverLobby: terminalInputBase{
    class PF {
    public:
        static void setLobbyMainTwoOrMorePlayers();
        static void setGameMain();

    };
    std::map<int, std::tuple<std::string, std::unique_ptr<session<serverLobby, true>>>> players;
    std::map<int, std::unique_ptr<session<serverLobby, true>>> yetToBePromotedSession;
    int maxID = 0;

    std::string joinedPlayerName;
    serverListener& serverlistener;
    asio::io_context& io;

    std::unique_ptr<serverChat> chatManagerPtr;
    std::unique_ptr<serverGetAway> serverGetAwayPtr;

    inputType inputTypeExpected;
    bool serverOnly = true;
public:
    bool gameStarted = false;
    serverLobby(serverListener& serverlistener_, asio::io_context& io_, bool serverOnly_);
    void shutDown();

    void newConnectionReceived(asio::ip::tcp::socket sock);
   // int join(std::shared_ptr<session<serverLobby, true>> lobbySession);

    void managementJoin(int excitedSessionId, const std::string &playerNameFinal);

    void sendPLAYERJOINEDToAllExceptOne(int excitedSessionId);

    void sendPLAYERLEFTToAllExceptOne(int excitedSessionId);

    void leave(int id);

    void setInputType(inputType type);

    void input(std::string inputString, inputType inputReceivedType) override;

    void packetReceivedFromNetwork(std::istream &in, int receivedPacketSize, int sessionId);

    void gameExitFinished();

    void sendPLAYERLEFTDURINGGAMEToAllExceptOne(int id);

    void startTheGame();
};


#endif //GETAWAY_SERVERLOBBY_HPP
