
#ifndef GETAWAY_GETAWAYTEST_HPP
#define GETAWAY_GETAWAYTEST_HPP

#include "inferior.hpp"
#include <vector>

enum class clientStages{
    START,
    HOME,
    FIND_LOCAL_SERVERS,
    FIND_LOCAL_SERVERS_UPDATED,
    ENTER_YOUR_NAME_OR_USE_DEFAULT_TO_JOIN_SERVER,
    LOBBY,
    GAME_DORMANT,
    GAME_PERFORM_TURN
};

enum class serverStages{
    START,
    HOME,
    ENTER_SERVER_NAME_OR_USE_DEFAULT_TO_JOIN_SERVER,
    LOBBY_DORMANT,
    LOBBY_START_GAME,
    GAME
};

class GetAwayTest {
    constexpr static const char serverApp[] = "../GetAway",
            clientApp[] = "../Client/Client";

    asio::io_context& io;
    inferior<GetAwayTest, false, serverApp, serverApp> serverInferior;
    std::vector<inferior<GetAwayTest, true, clientApp, clientApp>> clientInferiors;
    constexpr static const int numberOfClients = 4;
    constexpr static const int waitForOutputSeconds = 2;
    std::vector<std::string> clientNames;
    std::vector<asio::steady_timer> clientTimers;
    asio::steady_timer serverTimer;
    std::vector<clientStages> currentClientStages;
    serverStages currentServerStage;
public:
    explicit GetAwayTest(asio::io_context& io_);
    void outputReceivedFromInferior(std::string str);
    void outputReceivedFromInferior(std::string str, int id);
    void serverTimerExpiredBeforeOutputWasReceived();
    void clientTimerExpiredBeforeOutputWasReceived(int id);
};


#endif //GETAWAY_GETAWAYTEST_HPP
