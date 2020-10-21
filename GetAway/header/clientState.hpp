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
class clientState
{
    // This simple method of tracking sessions only works with an implicit strand (i.e. a single-threaded server)
    std::unordered_set<clientLobbySession*> sessions;
    std::vector<std::string> gamePlayers;
    std::chrono::seconds time_per_turn = std::chrono::seconds(60);

    int classSendSize =0;

    friend std::ostream& operator<<(std::ostream& out, clientState& state){
        out << state.gamePlayers.size() << std::endl;
        for(auto & gamePlayer : state.gamePlayers){
            out << gamePlayer << std::endl;
        }
        out << state.time_per_turn.count() << std::endl;
        return out;
    }

    friend std::istream& operator>>(std::istream& in, clientState& state){
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
    clientState();

    int getClassSendSize() const;
    void setClassSendSize(int size);
    void join  (clientLobbySession& session, const std::string& playerName);
    void leave (clientLobbySession& session);

    void stateSend  (std::string message, clientLobbySession* session);

};

#endif
