#include <algorithm>
#include <utility>
#include "clientLobbyManager.hpp"
#include "messageTypeEnums.hpp"
clientLobbyManager::clientLobbyManager(){
    messageTypeExpected.resize((int)lobbyMessageType::ENUMSIZE);
    messageTypeExpected[0] = lobbyMessageType::SELFANDSTATE;
}

std::istream &operator>>(std::istream &in, clientLobbyManager &manager) {
    lobbyMessageType messageTypeReceived;
    in.read(reinterpret_cast<char*>(&messageTypeReceived), sizeof(lobbyMessageType));
    if(std::find(manager.messageTypeExpected.begin(), manager.messageTypeExpected.end(), messageTypeReceived) != manager.messageTypeExpected.end()){
        std::cout<<"Unexpected Packet Type Received in class clientLobbyManager"<<std::endl;
    }
    switch(messageTypeReceived){
        case lobbyMessageType::SELFANDSTATE: {
            //STEP 1;
            int id;
            in.read(reinterpret_cast<char*>(&id),sizeof(id));
            manager.id = id;
            //TODO
            char arr[61]; //This constant will be fed from somewhere else but one is added.
            in.getline(arr, 61);
            std::string playerName(arr);
            //TODO
            //Action on if a new playerName is assigned.
            manager.playerName = std::move(playerName);

            //STEP 2
            int size;
            in.read(reinterpret_cast<char*>(&size), sizeof(size));
            for(int i=0; i<size; ++i){
                int playersId = 0;
                in.read(reinterpret_cast<char*>(&playersId), sizeof(playersId));
                in.getline(arr, 61);
                playerName = std::string(arr);
                manager.gamePlayers.emplace(playersId, playerName);
            }
            manager.messageTypeExpected[0] = lobbyMessageType::CHATMESSAGE;
            manager.messageTypeExpected[1] = lobbyMessageType::UPDATE;
            manager.clientLobbySession->receiveMessage();
            break;
        }
        case lobbyMessageType::UPDATE:{
            int playersId = 0;
            in.read(reinterpret_cast<char*>(&playersId), sizeof(playersId));
            char arr[61];
            in.getline(arr, 61);
            std::string playerName(arr);
            manager.gamePlayers.emplace(playersId, playerName);
            break;
        }
        case lobbyMessageType::CHATMESSAGE: {
            int messageSenderId;
            in.read(reinterpret_cast<char*>(&messageSenderId), sizeof(messageSenderId));
            char arr[manager.receivedPacketSize - 8];
            in.getline(arr, manager.receivedPacketSize - 8);
            //TODO
            //Do something with the received message.
            manager.chatMessage = std::string(arr);
            break;
        }
    }
    return in;
}

std::ostream &operator<<(std::ostream &out, clientLobbyManager &manager) {
    lobbyMessageType t = lobbyMessageType::CHATMESSAGE;
    out.write(reinterpret_cast<char*>(&t), sizeof(t));
    //STEP 2;
    out << manager.chatMessage << std::endl;
    return out;
}

void clientLobbyManager::join(std::shared_ptr<session<clientLobbyManager>> clientLobbySession_) {
    clientLobbySession = std::move(clientLobbySession_);
    messageTypeExpected.clear();
    messageTypeExpected[0] = lobbyMessageType::SELFANDSTATE;
    clientLobbySession->receiveMessage();
}