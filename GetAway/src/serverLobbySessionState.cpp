#include "serverLobbySessionState.hpp"
#include "serverLobbySession.hpp"


serverLobbySessionState::
serverLobbySessionState()= default;
int
serverLobbySessionState::
join(serverLobbySession& session, const std::string& playerName)
{
    auto tup = std::tuple(std::cref(playerName),std::ref(session));
    int id;
    if(gameData.empty())
    {
        id = 0;
        gameData.emplace(id,tup);
        gameData.insert(std::make_pair(id,tup));
    }
    else{
        id = gameData.rbegin()->first + 1;
        gameData.insert(std::make_pair(id, tup));
    }
    return id;
}

void
serverLobbySessionState::
leave(int id)
{
    gameData.erase(gameData.find(id));
}

//TODO
//When all other communication is being handled by the serverLobbySession
//then this broadcast should also be handled by the serverLobbySession
//but it is being handled by the serverLobbySessionState. So, basically
//state and session classes need a merger.


std::ostream &operator<<(std::ostream &out, serverLobbySessionState &state) {
    out << state.gameData.size() << std::endl;
    for(auto & gamePlayer : state.gameData){
        out << gamePlayer.first << std::endl;
        out << get<0>(gamePlayer.second).get() << std::endl;
    }
    out << state.timePerTurn.count() << std::endl;
    return out;
}

int serverLobbySessionState::getClassWriteSize() {
    int numbOfPlayers = gameData.size();
    int size = 0;
    for(auto& player: gameData){
        size += std::get<0>(player.second).get().size();
    }
    size += (numbOfPlayers * 5); //4 for int id and one for \n.
    return size;
}

void serverLobbySessionState::broadcastState() {
    for(auto& player: gameData){
        std::get<1>(player.second).get().writeState();
    }
}
//Don't need this class as client won't exactly read this data structure.
//I don't have to provide all the clientLobbySessions to the client.
//Only their id's matter.
/*std::istream& operator>>(std::istream& in, serverLobbySessionState& state)
{
    int size;
    in >> size;
    in.ignore();
    for(int i=0;i<size;++i){
        int id;
        in >> id;
        in.ignore();
//TODO
        char arr[61]; //This constant will be fed from somewhere else //
        in.getline(arr,61);
        std::string str(arr);
        state.gameData.insert(std::make_pair(id,str));
    }
    return in;
}*/
