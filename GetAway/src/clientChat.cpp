
#include <cassert>
#include "clientChat.hpp"
#include "clientLobby.hpp"
#include "clientGetAway.hpp"
#include "sati.hpp"

clientChat::clientChat(clientLobby& lobbyManager_, const std::map<int, std::string>& players_, const std::string& playerName_, int myId_):
        lobbyManager{lobbyManager_}, playerName{playerName_}, players{players_}, myId{myId_}
{
}

void clientChat::packetReceivedFromNetwork(std::istream &in, int receivedPacketSize) {
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
    PF::addAccumulate(players.find(senderId)->second,
                      std::string(arr));
#if defined(_WIN32) || defined(_WIN64)
    delete[] arr;
#endif
}

void clientChat::input(std::string inputString, inputType inputReceivedType_){
    if(inputReceivedType_ == inputType::MESSAGESTRING){
        if(inputString.empty()){
            if(lobbyManager.gameStarted){
                lobbyManager.setBaseAndInputTypeFromclientChatMessageOfGamePtr();
            }else{
                lobbyManager.setBaseAndInputTypeFromclientChatMessage();
            }
        }else{
            chatMessageString = std::move(inputString);
            chatMessageInt = myId;
            sendCHATMESSAGE();
            if(lobbyManager.gameStarted){
                lobbyManager.setBaseAndInputTypeFromclientChatMessageOfGamePtr();
            }else{
                lobbyManager.setBaseAndInputTypeFromclientChatMessage();
            }
        }
    }
}
void clientChat::sendCHATMESSAGEHandler(){
    PF::addAccumulate(players.find(chatMessageInt)->second, chatMessageString);
}

namespace clientChatManagerSendCHATMESSAGE{
    clientChat* chatManager;
    void func(){
        chatManager->sendCHATMESSAGEHandler();
    }
}

void clientChat::sendCHATMESSAGE(){
    std::ostream& out = lobbyManager.clientLobbySession.out;
    //STEP 1;
    out.write(reinterpret_cast<const char*>(&constants::mtcMessage), sizeof(constants::mtcMessage));
    //STEP 2;
    out.write(reinterpret_cast<char *>(&myId), sizeof(myId));
    //STEP 3;
    out << chatMessageString << std::endl;

    clientChatManagerSendCHATMESSAGE::chatManager = this;
    lobbyManager.clientLobbySession.sendMessage(clientChatManagerSendCHATMESSAGE::func);
}

void clientChat::setBaseAndInputTypeForMESSAGESTRING(){
    PF::setInputStatementAccumulate();
    sati::getInstance()->setBaseAndInputType(this, inputType::MESSAGESTRING);
}