
#include <sati.hpp>
#include "clientRoomManager.hpp"
#include "messageTypeEnums.hpp"
#include "clientHome.hpp"

clientRoomManager::clientRoomManager(asio::io_context& io_): io{io_} {

}

void clientRoomManager::join(std::shared_ptr<session<clientRoomManager>> clientRoomSession_) {
    clientRoomSession = std::move(clientRoomSession_);
    messageTypeExpected.clear();
    messageTypeExpected.emplace_back(mtr::SELFANDSTATE);
    clientRoomSession->receiveMessage();
}

void clientRoomManager::packetReceivedFromNetwork(std::istream &in, int receivedPacketSize) {

    mtc messageTypeReceived;
    //STEP 1;
    in.read(reinterpret_cast<char*>(&messageTypeReceived), sizeof(messageType));
    if(gameStarted && messageTypeReceived == mtc::GAME){
        lobbyManager->packetReceivedFromNetwork(in, receivedPacketSize);
    }else if(messageTypeReceived == mtc::MESSAGE){
        lobbyManager->packetReceivedFromNetwork(in, receivedPacketSize);
    }else if(messageTypeReceived == mtc::ROOM){
        mtr innerMessageType;
        in.read(reinterpret_cast<char*>(&innerMessageType), sizeof(innerMessageType));
        if(std::find(messageTypeExpected.begin(), messageTypeExpected.end(), innerMessageType) == messageTypeExpected.end()){
            resourceStrings::print("Unexpected Packet Type Received in class clientRoomManager."
                                   "Packet not present in vectory messageTypeExpected.\r\n");
        }else {
            switch (innerMessageType) {
                //STEP 1;
                case mtr::SELFANDSTATE: {
                    //STEP 2
                    in.read(reinterpret_cast<char *>(&myId), sizeof(myId));
                    //TODO
                    char arr[61]; //This constant will be fed from somewhere else but one is added.
                    //STEP 3
                    in.getline(arr, 61);
                    //TODO
                    //Action on if a new playerName is assigned.
                    playerName = std::string(arr);

                    int size;
                    //STEP 4
                    in.read(reinterpret_cast<char *>(&size), sizeof(size));
                    for (int i = 0; i < size; ++i) {
                        int playersId = 0;
                        //STEP 5
                        in.read(reinterpret_cast<char *>(&playersId), sizeof(playersId));
                        //STEP 6
                        in.getline(arr, 61);
                        playerName = std::string(arr);
                        players.emplace(playersId, playerName);
                    }
                    SELFANDSTATEReceived();
                    break;
                }
                    //STEP 1;
                case mtr::PLAYERJOINED: {
                    int playerId = 0;
                    //STEP 2;
                    in.read(reinterpret_cast<char *>(&playerId), sizeof(playerId));
                    char arr[61];
                    //STEP 3;
                    in.getline(arr, 61);
                    players.emplace(playerId, std::string(arr));
                    PLAYERJOINEDOrPLAYERLEFTReceived();
                    break;
                }
                case mtr::PLAYERLEFT: {
                    int playerId = 0;
                    in.read(reinterpret_cast<char *>(&playerId), sizeof(playerId));
                    players.erase(players.find(playerId));
                    PLAYERJOINEDOrPLAYERLEFTReceived();
                    break;
                }
                default: {
                    resourceStrings::print("Unknown Packet Type Received in class serverRoomManager."
                                           "Packet does not match of any type of mtr.\r\n");
                    break;
                }
            }
        }
    }
    else{
        resourceStrings::print("Unexpected Packet Type Received in class serverRoomManager\r\n");
    }
}

void clientRoomManager::SELFANDSTATEReceived(){
    messageTypeExpected.clear();
    messageTypeExpected.emplace_back(mtr::PLAYERJOINED);
    messageTypeExpected.emplace_back(mtr::PLAYERLEFT);
    setInputType(inputType::OPTIONSELECTIONINPUTLOBBY);
    sati::getInstance()->setBase(this, appState::LOBBY);
    lobbyPF::addOrRemovePlayer(players);
    lobbyPF::setInputStatementHomeAccumulate();
}

void clientRoomManager::PLAYERJOINEDOrPLAYERLEFTReceived(){
    lobbyPF::addOrRemovePlayer(players);
    lobbyPF::setInputStatementHomeAccumulate();
}

//Before refactor, lines of this function = 123;
void clientRoomManager::input(std::string inputString, inputType inputReceivedType) {
    if(inputReceivedType == inputTypeExpected){
        switch (inputReceivedType) {
            case inputType::OPTIONSELECTIONINPUTLOBBY:
            {
                int input;
                if(constants::inputHelper(inputString, 1, 3, inputType::OPTIONSELECTIONINPUTLOBBY, inputType::OPTIONSELECTIONINPUTLOBBY,
                                          input)){
                    if(input == 1){
                        messagePF::setInputStatementAccumulate();
                        setInputType(inputType::MESSAGESTRING);
                    }
                    else if(input== 2){
                        exitApplication();
                        std::make_shared<clientHome>(io)->run();
                    }else{
                        exitApplication();
                    }
                }
                break;
            }
        }
    }
    else{
        resourceStrings::print("InputReceived Type not same as Input Received Type Expected\r\n");
    }
}

void clientRoomManager::exitApplication(){
    clientRoomSession->sock.shutdown(asio::socket_base::shutdown_both);
    clientRoomSession->sock.close();
    clientRoomSession.reset();
}

void clientRoomManager::setInputType(inputType inputType) {
    inputTypeExpected = inputType;
    sati::getInstance()->setInputType(inputType);
}

void clientRoomManager::leaveGame(){
    exitApplication();
    std::make_shared<clientHome>(clientHome(io))->run();
}

void clientRoomManager::exitApplicationAmidGame(){
    exitApplication();
}