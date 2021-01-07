
#include <sati.hpp>
#include "clientRoomManager.hpp"
#include "messageTypeEnums.hpp"
#include "clientHome.hpp"
#include "clientLobbyManager.hpp"

clientRoomManager::clientRoomManager(asio::io_context& io_): io{io_} {
}

void clientRoomManager::join(std::shared_ptr<session<clientRoomManager>> clientRoomSession_) {
    clientRoomSession = std::move(clientRoomSession_);
    clientRoomSession->receiveMessage();
}

void clientRoomManager::packetReceivedFromNetwork(std::istream &in, int receivedPacketSize) {

    mtc messageTypeReceived;
    //STEP 1;
    in.read(reinterpret_cast<char*>(&messageTypeReceived), sizeof(messageType));
    if(messageTypeReceived == mtc::GAME){
        if(!gameStarted){
            lobbyManager = std::make_shared<clientLobbyManager>(*this, playerName, players, in, myId);
            gameStarted = true;
        }else{
            lobbyManager->packetReceivedFromNetwork(in, receivedPacketSize);
        }
    }else if(messageTypeReceived == mtc::MESSAGE){
        chatManager->packetReceivedFromNetwork(in, receivedPacketSize);
    }else if(messageTypeReceived == mtc::ROOM){
        mtr innerMessageType;
        in.read(reinterpret_cast<char*>(&innerMessageType), sizeof(innerMessageType));
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
                chatManager = std::make_shared<clientChatManager>(*this, players, playerName, myId);
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
            case mtr::PLAYERLEFTDURINGGAME:{
                int playerLeftId;
                in.read(reinterpret_cast<char *>(&playerLeftId), sizeof(playerLeftId));
                gameFinished();
                resourceStrings::print("Player Left During Game. Game Ended.\r\n");
                break;
            }
            default: {
                resourceStrings::print("Unknown Packet Type Received in class clientRoomManager."
                                       "Packet does not match of any type of mtr.\r\n");
                break;
            }
        }
    }
    else{
        resourceStrings::print("Unexpected Packet Type Received in class clientRoomManager "
                               "Packet Type Not Present In Enum mtc\r\n");
    }
    clientRoomSession->receiveMessage();
}

void clientRoomManager::SELFANDSTATEReceived(){
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
    int input;
    if(constants::inputHelper(inputString, 1, 3, inputType::OPTIONSELECTIONINPUTLOBBY, inputType::OPTIONSELECTIONINPUTLOBBY,
                              input)){
        if(input == 1){
            messagePF::setInputStatementAccumulate();
            sati::getInstance()->setBaseAndInputType(chatManager.get(), inputType::MESSAGESTRING);
        }
        else if(input== 2){
            exitApplication(true);
        }else{
            exitApplication(false);
        }
    }
}

void clientRoomManager::gameFinished(){
    gameStarted = false;
    sati::getInstance()->setBase(this, appState::LOBBY);

    lobbyPF::addOrRemovePlayer(players);
    lobbyPF::setInputStatementHomeAccumulate();
    setInputType(inputType::OPTIONSELECTIONINPUTLOBBY);
    lobbyManager.reset();
}

void clientRoomManager::exitApplication(bool backToHome){
    clientRoomSession->sock.shutdown(asio::socket_base::shutdown_both);
    clientRoomSession->sock.close();
    clientRoomSession.reset();
    if(backToHome)
        std::make_shared<clientHome>(clientHome(io))->run();
}

void clientRoomManager::setInputType(inputType inputType) {
    inputTypeExpected = inputType;
    sati::getInstance()->setInputType(inputType);
}