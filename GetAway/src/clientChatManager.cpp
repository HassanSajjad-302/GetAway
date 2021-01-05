
#include <messagePF.hpp>
#include <cassert>
#include "lobbyPF.hpp"
#include "clientChatManager.hpp"
#include "sati.hpp"

clientChatManager::clientChatManager(clientRoomManager& roomManager_, const std::map<int, std::string>& players_, const std::string& playerName_, int id):
        roomManager{roomManager_}, playerName{playerName_}, players{players_}, myId{id}
{
}

void clientChatManager::packetReceivedFromNetwork(std::istream &in, int receivedPacketSize) {
    //STEP 1
    int senderId;
    in.read(reinterpret_cast<char*>(&senderId), sizeof(senderId));
    assert(senderId != myId);
    int arrSize = (receivedPacketSize - 8) + 1; //4 for packety type enum and 4 for the id and 1 for getline
#if defined(_WIN32) || defined(_WIN64)
    char* arr = new char[arrSize];
#endif
#ifdef __linux__
    char arr[arrSize];
#endif
    //STEP 2
    in.getline(arr, receivedPacketSize - 8);
    messagePF::addAccumulate(players.find(senderId)->second,
                             std::string(arr));
#if defined(_WIN32) || defined(_WIN64)
    delete[] arr;
#endif
}

void clientChatManager::input(const std::string& inputString){
    if(inputString.empty()){
        lobbyPF::setInputStatementHomeAccumulate();
        if(roomManager.gameStarted){
            roomManager.lobbyManager->setBaseAndInputTypeFromclientChatMessage();
        }else{
            sati::getInstance()->setBaseAndInputType(&roomManager,
                                                     inputType::OPTIONSELECTIONINPUTLOBBY);
        }
    }else{
        chatMessageString = std::move(inputString);
        chatMessageInt = myId;
        sendCHATMESSAGE();
        lobbyPF::setInputStatementHomeAccumulate();
        if(roomManager.gameStarted){
            roomManager.lobbyManager->setBaseAndInputTypeFromclientChatMessage();
        }else{
            sati::getInstance()->setBaseAndInputType(&roomManager,
                                                     inputType::OPTIONSELECTIONINPUTLOBBY);
        }
    }
}
void clientLobbyManager::sendCHATMESSAGEHandler(){
    messagePF::addAccumulate(players.find(chatMessageInt)->second, chatMessageString);
}

namespace clientChatManagerSendCHATMESSAGE{
    clientChatManager* chatManager;
    void func(){
        chatManager->sendCHATMESSAGE();
    }
}
void clientChatManager::sendCHATMESSAGE(){
    std::ostream& out = roomManager.clientRoomSession->out;
    //STEP 1;
    messageType t = messageType::CHATMESSAGE;
    out.write(reinterpret_cast<char*>(&t), sizeof(t));
    //STEP 2;
    out.write(reinterpret_cast<char *>(&myId), sizeof(myId));
    //STEP 3;
    out << chatMessageString << std::endl;

    roomManager.clientRoomSession->sendMessage(clientChatManagerSendCHATMESSAGE::func);
}
