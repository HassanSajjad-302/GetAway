#include "clientLobbyManager.hpp"

int clientLobbyManager::getClassSendSize() const {
    return 0;
}

clientLobbyManager::clientLobbyManager() = default;

std::istream &operator>>(std::istream &in, clientLobbyManager &state) {
    in.read(reinterpret_cast<char*>(state.id),sizeof(state.id));
    //TODO
    char arr[61]; //This constant will be fed from somewhere else but one is added.
    in.getline(arr, 61);
    std::string playerName(arr);
    state.playerName = std::move(playerName);

    int size;
    in >> size;
    for(int i=0; i<size; ++i){
        int id = 0;
        in.read(reinterpret_cast<char*>(id),sizeof(id));
        in.getline(arr, 61);
        playerName = std::string(arr);
        state.gamePlayers.emplace(id, playerName);
    }
    return in;
}