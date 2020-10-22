#ifndef CPPCON2018_SHARED_STATE_HPP
#define CPPCON2018_SHARED_STATE_HPP

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <iostream>
class clientLobbySession;

// Represents the shared server state
class clientLobbySessionState
{
    std::string password;
    std::string playerName;
    int classSendSize =0;

    friend std::ostream& operator<<(std::ostream& out, clientLobbySessionState& state){
        out << state.password << std::endl;
        out << state.playerName << std::endl;
        return out;
    }

public:
    explicit
    clientLobbySessionState(std::string playerName, std::string password);

    int getClassSendSize() const;
    void setClassSendSize(int size);
    void join  (clientLobbySession& session, const std::string& playerName);
    void leave (clientLobbySession& session);

    void stateSend  (std::string message, clientLobbySession* session);

};

#endif
