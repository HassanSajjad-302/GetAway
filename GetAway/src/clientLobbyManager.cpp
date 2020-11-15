#include <utility>
#include "clientLobbyManager.hpp"
#include "messageTypeEnums.hpp"
#include "sati.hpp"
clientLobbyManager::clientLobbyManager(){
    messageTypeExpected.resize((int)lobbyMessageType::ENUMSIZE);
    messageTypeExpected[0] = lobbyMessageType::SELFANDSTATE;
    spdlog::info("ClientLobbyManager Constructor Called");
}

std::istream &operator>>(std::istream &in, clientLobbyManager &manager) {
    lobbyMessageType messageTypeReceived;
    in.read(reinterpret_cast<char*>(&messageTypeReceived), sizeof(lobbyMessageType));
    if(std::find(manager.messageTypeExpected.begin(), manager.messageTypeExpected.end(), messageTypeReceived) != manager.messageTypeExpected.end()){
        std::cout<<"Unexpected Packet Type Received in class clientLobbyManager"<<std::endl;
    }
    switch(messageTypeReceived){
        //STEP 1;
        case lobbyMessageType::SELFANDSTATE: {
            int id;
            //STEP 2
            in.read(reinterpret_cast<char*>(&id),sizeof(id));
            manager.id = id;
            //TODO
            char arr[61]; //This constant will be fed from somewhere else but one is added.
            //STEP 3
            in.getline(arr, 61);
            std::string playerName(arr);
            //TODO
            //Action on if a new playerName is assigned.
            manager.playerName = std::move(playerName);

            int size;
            //STEP 4
            in.read(reinterpret_cast<char*>(&size), sizeof(size));
            for(int i=0; i<size; ++i){
                int playersId = 0;
                //STEP 5
                in.read(reinterpret_cast<char*>(&playersId), sizeof(playersId));
                //STEP 6
                in.getline(arr, 61);
                playerName = std::string(arr);
                manager.gamePlayers.emplace(playersId, playerName);
            }
            manager.messageTypeExpected[0] = lobbyMessageType::CHATMESSAGEID;
            manager.messageTypeExpected[1] = lobbyMessageType::PLAYERJOINED;
            manager.messageTypeExpected[2] = lobbyMessageType::PLAYERLEFT;
            manager.managementLobbyReceived();
            manager.clientLobbySession->receiveMessage();

            break;
        }
        //STEP 1;
        case lobbyMessageType::PLAYERJOINED:{
            int playerId = 0;
            //STEP 2;
            in.read(reinterpret_cast<char*>(&playerId), sizeof(playerId));
            char arr[61];
            //STEP 3;
            in.getline(arr, 61);
            std::string playerName(arr);
            manager.gamePlayers.emplace(playerId, playerName);
            manager.managementLobbyReceived();
            manager.clientLobbySession->receiveMessage();
            break;
        }
        case lobbyMessageType::PLAYERLEFT:{
            int playerId = 0;
            in.read(reinterpret_cast<char*>(&playerId), sizeof(playerId));
            manager.gamePlayers.erase(manager.gamePlayers.find(playerId));
            manager.managementLobbyReceived();
            manager.clientLobbySession->receiveMessage();
            break;
        }
        //STEP 1;
        case lobbyMessageType::CHATMESSAGEID: {
            //STEP 2;
            in.read(reinterpret_cast<char*>(&manager.chatMessageInt), sizeof(manager.chatMessageInt));
            assert(manager.chatMessageInt != manager.id);
            int arrSize = (manager.receivedPacketSize - 8) + 1; //4 for packety type enum and 4 for the id and 1 for getline
            char arr[arrSize];
            //STEP 3;
            in.getline(arr, manager.receivedPacketSize - 8);
            manager.chatMessageString = std::string(arr);
            sati::getInstance()->messageBufferAppend(manager.gamePlayers.find(manager.chatMessageInt)->second
            + ": " +  manager.chatMessageString + "\r\n");
            manager.clientLobbySession->receiveMessage();
            break;
        }
        //STEP 1;
        case lobbyMessageType::GAMESTART
        :{
            int playerId = 0;
            //STEP 2;
            in.read(reinterpret_cast<char*>(&playerId), sizeof(playerId));
            char arr[61];
            //STEP 3;
            in.getline(arr, 61);
            std::string playerName(arr);
            manager.gamePlayers.emplace(playerId, playerName);
            manager.managementLobbyReceived();
            manager.clientLobbySession->receiveMessage();
            break;
        }
    }
    return in;
}

std::ostream &operator<<(std::ostream &out, clientLobbyManager &manager) {
    //STEP 1;
    lobbyMessageType t = lobbyMessageType::CHATMESSAGE;
    out.write(reinterpret_cast<char*>(&t), sizeof(t));
    //STEP 2;
    out.write(reinterpret_cast<char *>(&manager.id), sizeof(manager.id));
    //STEP 3;
    out << manager.chatMessageString << std::endl;
    return out;
}

void clientLobbyManager::join(std::shared_ptr<session<clientLobbyManager>> clientLobbySession_) {
    clientLobbySession = std::move(clientLobbySession_);
    messageTypeExpected.clear();
    messageTypeExpected[0] = lobbyMessageType::SELFANDSTATE;
    clientLobbySession->receiveMessage();
    managementNextAction();
}

void clientLobbyManager::uselessWriteFunction(){
    sati::getInstance()->messageBufferAppend(gamePlayers.find(chatMessageInt)->second + ": " +  chatMessageString + "\r\n");
}


void clientLobbyManager::managementLobbyReceived(){
    std::string roundBufferBefore = "Players in Lobby Are\r\n";
    for(auto& player: gamePlayers) {
        roundBufferBefore += player.second;
        roundBufferBefore += "\r\n";
    }
    roundBufferBefore += "\r\n";
    sati::getInstance()->roundBufferBeforeChanged(roundBufferBefore);
}

void clientLobbyManager::managementNextAction(){
    std::string toPrint = "1)Send Message 2)Exit\r\n\n";
    sati::getInstance()->inputStatementBufferChanged(toPrint, false);
    sati::getInstance()->setIntHandlerAndConstraints(this, 1, 2);
}

void clientLobbyManager::inputInt(int input) {
    if(input == 1){
        std::string toPrint = "Please Type The Message \r\n\n";
        sati::getInstance()->inputStatementBufferChanged(toPrint, false);
        sati::getInstance()->setStringHandlerAndConstraints(this);
    }
    if(input == 2){
        sati::getInstance()->inputStatementBufferChanged("Exiting\r\n",true);
        exitGame();
    }
}

void clientLobbyManager::exitGame(){
    clientLobbySession->sock.shutdown(net::socket_base::shutdown_both);
    clientLobbySession->sock.close();
    clientLobbySession.reset();
}

void clientLobbyManager::inputString(std::string input) {
    chatMessageString = std::move(input);

    //TODO
    //This is an error if before the message write finishes, player receives the message
    chatMessageInt = id;
    clientLobbySession->sendMessage(&clientLobbyManager::uselessWriteFunction);
    std::string toPrint = "1)Send Message 2)Exit\r\n\n";
    sati::getInstance()->inputStatementBufferChanged(toPrint, false);
    sati::getInstance()->setIntHandlerAndConstraints(this, 1, 2);
}

clientLobbyManager::~clientLobbyManager() {
    sati::getInstance()->printExitMessage("clientLobbyManagerDestructor Called");
    spdlog::info("ClientLobbyManager Destructor Called");
}
