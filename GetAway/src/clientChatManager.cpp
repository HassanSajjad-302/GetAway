
#include <messagePF.hpp>
#include <cassert>
#include "lobbyPF.hpp"
#include "clientChatManager.hpp"

clientChatManager::clientChatManager(const std::map<int, std::string>& players_, const std::string& playerName_, int id):
        playerName{playerName_}, players{players_}, myId{id}
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
        gameStarted ? setInputTypeGameInt() : setInputType(inputType::LOBBYINT);
    }else{
        chatMessageString = std::move(inputString);
        chatMessageInt = id;
        sendCHATMESSAGE();
        lobbyPF::setInputStatementHomeAccumulate();
        gameStarted ? setInputTypeGameInt() : setInputType(inputType::LOBBYINT);
    }
}

void clientChatManager::sendCHATMESSAGE(){
    std::ostream& out = clientLobbySession->out;
    //STEP 1;
    messageType t = messageType::CHATMESSAGE;
    out.write(reinterpret_cast<char*>(&t), sizeof(t));
    //STEP 2;
    out.write(reinterpret_cast<char *>(&id), sizeof(id));
    //STEP 3;
    out << chatMessageString << std::endl;

    clientLobbySession->sendMessage(&clientLobbyManager::uselessWriteFunctionCHATMESSAGE);
}
