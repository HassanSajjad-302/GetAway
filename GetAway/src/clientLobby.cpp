
#include "sati.hpp"
#include "clientLobby.hpp"
#include "messageTypeEnums.hpp"
#include "clientHome.hpp"
#include "clientGetAway.hpp"

clientLobby::clientLobby(clientSession<clientLobby, false, asio::io_context&, std::string>& clientLobbySession_,
                         asio::io_context& io_, std::string playerName):
clientLobbySession{clientLobbySession_}, io{io_}, myName(std::move(playerName)) {
}

void clientLobby::run() {
    mtc sendingType = mtc::LOBBY;
    clientLobbySession.out.write(reinterpret_cast<char *>(&sendingType), sizeof(sendingType));
    clientLobbySession.out << myName << std::endl;
    clientLobbySession.sendMessage();
    clientLobbySession.receiveMessage();
}

void clientLobby::packetReceivedFromNetwork(std::istream &in, int receivedPacketSize) {

    mtc messageTypeReceived;
    //STEP 1;
    in.read(reinterpret_cast<char*>(&messageTypeReceived), sizeof(messageType));
    if(messageTypeReceived == mtc::GAME){
        if(!gameStarted){
            clientGetAwayPtr = std::make_unique<clientGetAway>(*this, myName, players, in, myId);
            gameStarted = true;
        }else{
            clientGetAwayPtr->packetReceivedFromNetwork(in, receivedPacketSize);
        }
    }else if(messageTypeReceived == mtc::MESSAGE){
        clientChatPtr->packetReceivedFromNetwork(in, receivedPacketSize);
    }else if(messageTypeReceived == mtc::LOBBY){
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
                //Action on if a new myName is assigned.
                myName = std::string(arr);

                int size;
                //STEP 4
                in.read(reinterpret_cast<char *>(&size), sizeof(size));
                for (int i = 0; i < size; ++i) {
                    int playersId = 0;
                    //STEP 5
                    in.read(reinterpret_cast<char *>(&playersId), sizeof(playersId));
                    //STEP 6
                    in.getline(arr, 61);
                    myName = std::string(arr);
                    players.emplace(playersId, myName);
                }
                SELFANDSTATEReceived();
                clientChatPtr = std::make_unique<clientChat>(*this, players, myName, myId);
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
                resourceStrings::print("Unknown Packet Type Received in class clientLobby."
                                       "Packet does not match of any type of mtr.\r\n");
                break;
            }
        }
    }
    else{
        resourceStrings::print("Unexpected Packet Type Received in class clientLobby "
                               "Packet Type Not Present In Enum mtc\r\n");
    }
    clientLobbySession.receiveMessage();
}

void clientLobby::SELFANDSTATEReceived(){
    setInputType(inputType::OPTIONSELECTIONINPUTLOBBY);
    sati::getInstance()->setBase(this, appState::LOBBY);
    PF::addOrRemovePlayerAccumulate(players);
}

void clientLobby::PLAYERJOINEDOrPLAYERLEFTReceived(){
    PF::addOrRemovePlayerAccumulate(players);
}

//Before refactor, lines of this function = 123;
void clientLobby::input(std::string inputString, inputType inputReceivedType) {
    int input;
    if(constants::inputHelper(inputString, 1, 3, inputType::OPTIONSELECTIONINPUTLOBBY,
                              inputType::OPTIONSELECTIONINPUTLOBBY,input)){
        if(input == 1){
            clientChatPtr->setBaseAndInputTypeForMESSAGESTRING();
        }
        else if(input== 2){
            exitApplication(true);
        }else{
            exitApplication(false);
        }
    }
}

void clientLobby::gameFinished(){
    gameStarted = false;
    sati::getInstance()->setBase(this, appState::LOBBY);

    PF::addOrRemovePlayerAccumulate(players);
    setInputType(inputType::OPTIONSELECTIONINPUTLOBBY);
    clientGetAwayPtr.reset();
}

void clientLobby::exitApplication(bool backToHome){
    clientLobbySession.sock.shutdown(asio::socket_base::shutdown_both);
    clientLobbySession.sock.close();
    if(backToHome)
        std::make_shared<clientHome>(clientHome(io))->run();
}

void clientLobby::setInputType(inputType inputType) {
    inputTypeExpected = inputType;
    sati::getInstance()->setInputType(inputType);
}

void clientLobby::setBaseAndInputTypeFromclientChatMessage() {
    PF::setInputStatementHomeAccumulate();
    sati::getInstance()->setBaseAndInputType(this,
                                             inputType::OPTIONSELECTIONINPUTLOBBY);
}
