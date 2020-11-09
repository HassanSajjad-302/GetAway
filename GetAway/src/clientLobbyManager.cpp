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
            manager.messageTypeExpected[1] = lobbyMessageType::PLAYERJOINED;
            manager.messageTypeExpected[2] = lobbyMessageType::PLAYERLEFT;
            manager.managementLobbyReceived();
            manager.clientLobbySession->receiveMessage();

            break;
        }
        case lobbyMessageType::PLAYERJOINED:{
            int playersId = 0;
            in.read(reinterpret_cast<char*>(&playersId), sizeof(playersId));
            char arr[61];
            in.getline(arr, 61);
            std::string playerName(arr);
            manager.gamePlayers.emplace(playersId, playerName);
            break;
        }
        case lobbyMessageType::PLAYERLEFT:{
            int playersId = 0;
            in.read(reinterpret_cast<char*>(&playersId), sizeof(playersId));
            manager.gamePlayers.erase(manager.gamePlayers.find(playersId));
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

void clientLobbyManager::uselessWriteFunction(){

}


void clientLobbyManager::managementLobbyReceived(){
    std::cout<<"Players in Lobby Are" <<std::endl;
    for(auto& player: gamePlayers){
        std::cout<<player.second <<std::endl;
    }
}

void clientLobbyManager::managementNextAction(){
    int input = 0;
    std::cout<<"1)Send Message 3)Exit" <<std::endl;
    std::cin >> input;
    if(input == 1){
        std::string message;
        std::cout << "Type Message"<<std::endl;
        std::cin >> chatMessage;
        clientLobbySession->sendMessage(&clientLobbyManager::uselessWriteFunction);
    }
    if(input == 3){
        boost::system::error_code ec;
        clientLobbySession->sock.close(ec);
    }
}

