
#include "sati.hpp"
#include "clientLobby.hpp"
#include "messageTypeEnums.hpp"
#include "home.hpp"
#include "clientGetAway.hpp"

clientLobby::clientLobby(clientSession<clientLobby, false, asio::io_context&, std::string, serverListener*, bool>&
        clientLobbySession_, asio::io_context& io_, std::string playerName, serverListener* serverlistener_,
        bool isItClientOnly): clientLobbySession{clientLobbySession_}, io{io_}, myName(std::move(playerName)),
        listener(serverlistener_), clientOnly(isItClientOnly){
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
            clientGetAwayPtr = std::make_unique<clientGetAway>(*this, myName, players, in, myId, clientOnly);
            gameStarted = true;
        }else{
            clientGetAwayPtr->packetReceivedFromNetwork(in, receivedPacketSize);
        }
    }else if(messageTypeReceived == mtc::MESSAGE){
        clientChatPtr->packetReceivedFromNetwork(in, receivedPacketSize);
    }else if(messageTypeReceived == mtc::LOBBY){
        mtl innerMessageType;
        in.read(reinterpret_cast<char*>(&innerMessageType), sizeof(innerMessageType));
        switch (innerMessageType) {
            //STEP 1;
            case mtl::SELFANDSTATE: {
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
                if(clientOnly){
                    if(players.size() >= 2){
                        PF::inputStatement = "2)Send Message 3)Leave 4)Exit\r\n";
                    }else{
                        PF::inputStatement = "3)Leave 4)Exit\r\n";
                    }
                }else{
                    PF::inputStatement = "3)Close Server 4)Exit\r\n";
                }
                setInputType(inputType::OPTIONSELECTIONINPUTLOBBY);
                sati::getInstance()->setBase(this, appState::LOBBY);
                PF::addOrRemovePlayerAccumulate(players);
                clientChatPtr = std::make_unique<clientChat>(*this, players, myName, myId);
                break;
            }
                //STEP 1;
            case mtl::PLAYERJOINED: {
                int playerId = 0;
                //STEP 2;
                in.read(reinterpret_cast<char *>(&playerId), sizeof(playerId));
                char arr[61];
                //STEP 3;
                in.getline(arr, 61);
                players.emplace(playerId, std::string(arr));
                if(players.size() == 2){
                    if(clientOnly){
                        PF::inputStatement = "2)Send Message 3)Leave 4)Exit\r\n";
                    }else{
                        PF::inputStatement = "1)Start Game 2)Send Message 3)Close Server 4)Exit\r\n";
                    }
                }
                PF::addOrRemovePlayerAccumulate(players);
                break;
            }
            case mtl::PLAYERLEFT: {
                int playerId = 0;
                in.read(reinterpret_cast<char *>(&playerId), sizeof(playerId));
                players.erase(players.find(playerId));
                if(players.size() == 1){
                    if(clientOnly){
                        PF::inputStatement = "3)Leave 4)Exit\r\n";
                    }else{
                        PF::inputStatement = "3)Close Server 4)Exit\r\n";
                    }
                }
                PF::addOrRemovePlayerAccumulate(players);
                break;
            }
            case mtl::PLAYERLEFTDURINGGAME:{
                int playerLeftId;
                in.read(reinterpret_cast<char *>(&playerLeftId), sizeof(playerLeftId));
                gameStarted = false;
                players.erase(playerLeftId);
                if(players.size() == 1){
                    if(clientOnly){
                        PF::inputStatement = "3)Leave 4)Exit\r\n";
                    }else{
                        PF::inputStatement = "3)Close Server 4)Exit\r\n";
                    }
                }
                sati::getInstance()->setBase(this, appState::LOBBY);

                PF::addOrRemovePlayerAccumulate(players);
                setInputType(inputType::OPTIONSELECTIONINPUTLOBBY);
                clientGetAwayPtr.reset();
                resourceStrings::print("Player Left During Game. Game Ended.\r\n");
                //players.erase(playerLeftId);
                break;
            }
            default: {
                resourceStrings::print("Unknown Packet Type Received in class clientLobby."
                                       "Packet does not match of any type of mtl.\r\n");
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

//Before refactor, lines of this function = 123;
void clientLobby::input(std::string inputString, inputType inputReceivedType) {
    int input;
    if(clientOnly){
        if(players.size()>=2){
            if(constants::inputHelper(inputString, 2, 4, inputType::OPTIONSELECTIONINPUTLOBBY,
                                      inputType::OPTIONSELECTIONINPUTLOBBY,input)){
                if(input == 2){
                    clientChatPtr->setBaseAndInputTypeForMESSAGESTRING();
                }
                else if(input== 3){
                    exitApplication(true);
                }else{
                    exitApplication(false);
                }
            }
        }else{
            if(constants::inputHelper(inputString, 3, 4, inputType::OPTIONSELECTIONINPUTLOBBY,
                                      inputType::OPTIONSELECTIONINPUTLOBBY,input)){
                if(input== 3){
                    exitApplication(true);
                }else{
                    exitApplication(false);
                }
            }
        }
    }else{
        if(players.size()>=2){
            if(constants::inputHelper(inputString, 1, 4, inputType::OPTIONSELECTIONINPUTLOBBY,
                                      inputType::OPTIONSELECTIONINPUTLOBBY,input)){
                if(input == 1){
                    //Start Game
                    listener->startTheGame();
                }
                else if(input == 2){
                    clientChatPtr->setBaseAndInputTypeForMESSAGESTRING();
                }
                else if(input== 3){
                    exitApplication(true);
                }else{
                    exitApplication(false);
                }
            }
        }else{
            if(constants::inputHelper(inputString, 3, 4, inputType::OPTIONSELECTIONINPUTLOBBY,
                                      inputType::OPTIONSELECTIONINPUTLOBBY,input)){
                if(input== 3){
                    exitApplication(true);
                }else{
                    exitApplication(false);
                }
            }
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
    if(!clientOnly){
        listener->shutdown();
    }
    if(backToHome)
        std::make_shared<home>(home(io))->run();
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
