#ifndef GETAWAY_CLIENTLOBBYSESSIONSTATE_HPP
#define GETAWAY_CLIENTLOBBYSESSIONSTATE_HPP


//TODO
//A lot of code is getting repeated here in classes like
//serverLobbySession, serverTcpSession and similar classes of
//client side. These classes include communication code which
//is very simple. This code can be parameterized in templates
//form and placed in the state classes itself. This I will try in
//new branch after first at-least successfully completing and testing
//lobby session initialization and messaging.


#include <memory>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <iostream>
#include <map>
#include <tuple>
class serverLobbySession;
class serverLobbySessionState
{
    //TODO
    //This should be supplied by serverTcpSessionState. But currently
    //not doing this, as huge overall design change is expected.
    std::chrono::seconds timePerTurn = std::chrono::seconds(60);
    std::map<int, std::tuple<std::reference_wrapper<const std::string>,
    std::reference_wrapper<serverLobbySession>>> gameData;

    friend std::ostream& operator<<(std::ostream& out, serverLobbySessionState& state);
    //friend std::istream& operator>>(std::istream& in, serverLobbySessionState& state);


public:
    explicit
    serverLobbySessionState();

    int getClassWriteSize();
    int join  (serverLobbySession& session, const std::string& playerName);
    void leave (int id);

    void broadcastState();
};
#endif //GETAWAY_CLIENTLOBBYSESSIONSTATE_HPP
