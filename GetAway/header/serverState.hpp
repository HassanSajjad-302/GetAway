#ifndef CPPCON2018_SHARED_STATE_HPP
#define CPPCON2018_SHARED_STATE_HPP

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <iostream>
class serverLobbySession;

// Represents the shared server state
class serverState
{
    // This simple method of tracking sessions only works with an implicit strand (i.e. a single-threaded server)
    std::unordered_set<serverLobbySession*> sessions;
    std::vector<std::string> gamePlayers;
    std::chrono::seconds time_per_turn = std::chrono::seconds(60);

    int classSendSize =0;

    friend std::ostream& operator<<(std::ostream& out, serverState& state){
        out << state.gamePlayers.size() << std::endl;
        for(auto & gamePlayer : state.gamePlayers){
            out << gamePlayer << std::endl;
        }
        out << state.time_per_turn.count() << std::endl;
        return out;
    }

    friend std::istream& operator>>(std::istream& in, serverState& state){
        int size;
        in >> size;
        in.ignore();
        for(int i=0;i<size;++i){
            char arr[61]; //This constant will be fed from somewhere else
            in.getline(arr,61);
            std::string str(arr);
            state.gamePlayers.push_back(str);
        }
        return in;
    }


public:
    explicit
    serverState();

    int getClassSendSize() const;
    void setClassSendSize(int size);
    void join  (serverLobbySession& session, const std::string& playerName);
    void leave (serverLobbySession& session);

    void stateSend  (std::string message, serverLobbySession* session);

};

#endif
