
#ifndef GETAWAY_SERVERLOBBY_HPP
#define GETAWAY_SERVERLOBBY_HPP

#include <map>
#include <memory>
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"
#include "serverSession.hpp"
#include "terminalInputBase.hpp"
#include "serverGetAway.hpp"
#include "serverBluff.hpp"
#include "serverChat.hpp"
class serverListener;

//For game Foo, you will define std::unique_ptr<Foo> with other pointers of the game and all if conditions in member
//functions concerning constants::gamesEnum will be updated
class serverLobby: terminalInputBase{
    class PF {
    public:
        static void setLobbyMainTwoOrMorePlayers();
        static void setGameMain();

    };
    std::map<int, std::tuple<std::string, std::unique_ptr<serverSession<serverLobby>>>> players;
    std::map<int, std::unique_ptr<serverSession<serverLobby>>> yetToBePromotedSession;
    int maxID = 0;

    std::string joinedPlayerName;
    serverListener& serverlistener;
    asio::io_context& io;

    std::unique_ptr<serverChat> chatManagerPtr;

    std::unique_ptr<serverGetAway> serverGetAwayPtr;
    std::unique_ptr<serverBluff> serverBluffPtr;

    inputType inputTypeExpected;
    bool serverOnly = true;
    constants::gamesEnum gameSelected;
public:
    bool gameStarted = false;
    serverLobby(serverListener& serverlistener_, asio::io_context& io_, bool serverOnly_,
                constants::gamesEnum gameSelected);
    void shutDown();

    void newConnectionReceived(asio::ip::tcp::socket sock);
   // int join(std::shared_ptr<serverSession<serverLobby, true>> lobbySession);

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
