#include <algorithm>
#include <utility>
#include "clientLobbyManager.hpp"
#include "messageTypeEnums.hpp"
clientLobbyManager::clientLobbyManager(){
    messageExpected = true;
    messageTypeExpected[0] = lobbyMessageType::STATE;
}

std::istream &operator>>(std::istream &in, clientLobbyManager &state) {
    lobbyMessageType messageType;
    in.read(reinterpret_cast<char*>(messageType),sizeof(lobbyMessageType));
    if(state.messageExpected && std::find(state.messageTypeExpected.begin(), state.messageTypeExpected.end(), messageType) != state.messageTypeExpected.end()){
        std::cout<<"Unexpected Packet Type Received in class clientLobbyManager"<<std::endl;
    }
    switch(messageType){
        case lobbyMessageType::STATE: {
            int id;
            in.read(reinterpret_cast<char*>(id),sizeof(id));
            state.id = id;
            //TODO
            char arr[61]; //This constant will be fed from somewhere else but one is added.
            in.getline(arr, 61);
            std::string playerName(arr);
            //TODO
            //Action on if a new playerName is assigned.
            state.playerName = std::move(playerName);

            int size;
            in >> size;
            for(int i=0; i<size; ++i){
                int playersId = 0;
                in.read(reinterpret_cast<char*>(playersId), sizeof(playersId));
                in.getline(arr, 61);
                playerName = std::string(arr);
                state.gamePlayers.emplace(playersId, playerName);
            }
            state.messageTypeExpected[0] = lobbyMessageType::MESSAGE;
            state.messageTypeExpected[1] = lobbyMessageType::UPDATE;
            state.clientLobbySession->receiveMessage(&clientLobbyManager::emptyFunc);
            break;
        }
        case lobbyMessageType::UPDATE:{
            int playersId = 0;
            in.read(reinterpret_cast<char*>(playersId), sizeof(playersId));
            char arr[61];
            in.getline(arr, 61);
            std::string playerName(arr);
            state.gamePlayers.emplace(playersId, playerName);
            break;
        }
        case lobbyMessageType::MESSAGE: {
            int messageSenderId;
            in.read(reinterpret_cast<char*>(messageSenderId), sizeof(messageSenderId));
            char arr[state.clientLobbySession->receivedPacketSize - 8];
            in.getline(arr, state.clientLobbySession->receivedPacketSize - 8);
            //TODO
            //Do something with the received message.
            std::string message(arr);
            break;
        }
    }
    return in;
}

std::ostream &operator<<(std::ostream &out, clientLobbyManager &state) {

    return out;
}

void clientLobbyManager::emptyFunc(){

}
void clientLobbyManager::join(std::shared_ptr<session<clientLobbyManager>> clientLobbySession_) {
    clientLobbySession = std::move(clientLobbySession_);
}