
#include <cassert>
#include "clientChat.hpp"
#include "clientLobby.hpp"
#include "clientGetAway.hpp"
#include "sati.hpp"

clientChat::clientChat(clientLobby& lobby_, const std::map<int, std::string>& players_, const std::string& playerName_, int myId_):
        lobby{lobby_}, playerName{playerName_}, players{players_}, myId{myId_}
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
        if(!inputString.empty()){
            chatMessageString = std::move(inputString);
            chatMessageInt = myId;
            sendCHATMESSAGE();
        }
        lobby.setBaseAndInputTypeFromclientChatMessage();
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
    std::ostream& out = lobby.clientLobbySession.out;
    //STEP 1;
    out.write(reinterpret_cast<const char*>(&constants::mtcMessage), sizeof(constants::mtcMessage));
    //STEP 2;
    out << chatMessageString << std::endl;

    clientChatManagerSendCHATMESSAGE::chatManager = this;
    lobby.clientLobbySession.sendMessage(clientChatManagerSendCHATMESSAGE::func);
}

void clientChat::setBaseAndInputTypeForMESSAGESTRING(){
    PF::setInputStatementAccumulate();
    sati::getInstance()->setBaseAndInputType(this, inputType::MESSAGESTRING);
}