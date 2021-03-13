
#include "sati.hpp"
#include "clientLobby.hpp"
#include "messageTypeEnums.hpp"
#include "home.hpp"
#include "clientGetAway.hpp"
#include "clientBluff.hpp"

clientLobby::clientLobby(clientSession<clientLobby, asio::io_context&, std::string, serverListener*, bool,
                         constants::gamesEnum>& clientLobbySession_, asio::io_context& io_, std::string playerName,
                         serverListener* serverlistener_, bool isItClientOnly, constants::gamesEnum gameSelected_):
                         clientLobbySession{clientLobbySession_}, io{io_}, myName(std::move(playerName)),
                         listener(serverlistener_), clientOnly(isItClientOnly), gameSelected(gameSelected_){
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
            if(gameSelected == constants::gamesEnum::GETAWAY){
                clientGetAwayPtr = std::make_unique<clientGetAway>(*this, myName, players, in, myId, clientOnly);
            }else if(gameSelected == constants::gamesEnum::BLUFF){
                clientBluffPtr = std::make_unique<Bluff::clientBluff>(*this, myName, players, in, myId, clientOnly);
            }
            gameStarted = true;
        }else{
            if(gameSelected == constants::gamesEnum::GETAWAY){
                clientGetAwayPtr->packetReceivedFromNetwork(in, receivedPacketSize);
            }else if(gameSelected == constants::gamesEnum::BLUFF){
                clientBluffPtr->packetReceivedFromNetwork(in, receivedPacketSize);
            }
        }
    }else if(messageTypeReceived == mtc::MESSAGE){
        clientChatPtr->packetReceivedFromNetwork(in, receivedPacketSize);
    }else if(messageTypeReceived == mtc::LOBBY){
        mtl innerMessageType;
        in.read(reinterpret_cast<char*>(&innerMessageType), sizeof(innerMessageType));
        switch (innerMessageType) {
            //STEP 1;
            case mtl::SELFANDSTATE: {
                in.read(reinterpret_cast<char *>(&gameSelected), sizeof(gameSelected));
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
                        sati::getInstance()->inputStatement = "2)Send Message 3)Leave 4)Exit\r\n";
                    }else{
                        sati::getInstance()->inputStatement  = "3)Leave 4)Exit\r\n";
                    }
                }else{
                    sati::getInstance()->inputStatement  = "3)Close Server 4)Exit\r\n";
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
                        sati::getInstance()->inputStatement  = "2)Send Message 3)Leave 4)Exit\r\n";
                    }else{
                        sati::getInstance()->inputStatement  = "1)Start Game 2)Send Message 3)Close Server 4)Exit\r\n";
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
                        sati::getInstance()->inputStatement  = "3)Leave 4)Exit\r\n";
                    }else{
                        sati::getInstance()->inputStatement  = "3)Close Server 4)Exit\r\n";
                    }
                }
                PF::addOrRemovePlayerAccumulate(players);
                break;
            }
            case mtl::PLAYERLEFTDURINGGAME:{
                int playerLeftId;
                in.read(reinterpret_cast<char *>(&playerLeftId), sizeof(playerLeftId));

                players.erase(playerLeftId);
                if(players.size() == 1){
                    if(clientOnly){
                        sati::getInstance()->inputStatement  = "3)Leave 4)Exit\r\n";
                    }else{
                        sati::getInstance()->inputStatement  = "3)Close Server 4)Exit\r\n";
                    }
                }

                gameFinished();
                resourceStrings::print("Player Left During Game. Game Ended.\r\n");
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
    setInputStatementClientLobby();
    PF::addOrRemovePlayerAccumulate(players);
    setInputType(inputType::OPTIONSELECTIONINPUTLOBBY);
    if(gameSelected == constants::gamesEnum::GETAWAY){
        clientGetAwayPtr.reset();

    }else if(gameSelected == constants::gamesEnum::BLUFF){
        clientBluffPtr.reset();
    }
}

void clientLobby::exitApplication(bool backToHome){
    clientLobbySession.sock.shutdown(asio::socket_base::shutdown_both);
    clientLobbySession.sock.close();
    if(!clientOnly){
        if(gameStarted){
            listener->shutdown();
        }else{
            listener->closeAcceptorAndShutdown();
        }
    }
    if(backToHome)
        std::make_shared<home>(home(io))->run();
}

void clientLobby::setInputType(inputType inputType) {
    sati::getInstance()->setInputType(inputType);
}

void clientLobby::setInputStatementClientLobby(){
    if(clientOnly){
        if(players.size() >= 2){
            sati::getInstance()->inputStatement = "2)Send Message 3)Leave 4)Exit\r\n";
        }else{
            sati::getInstance()->inputStatement  = "3)Leave 4)Exit\r\n";
        }
    }else{
        if(players.size() >= 2){
            sati::getInstance()->inputStatement  = "1)Start Game 2)Send Message 3)Close Server 4)Exit\r\n";
        }else{
            sati::getInstance()->inputStatement  = "3)Close Server 4)Exit\r\n";
        }
    }
}

void clientLobby::setBaseAndInputTypeFromclientChatMessage() {
    if(gameStarted){
        if(gameSelected == constants::gamesEnum::GETAWAY){
            clientGetAwayPtr->setBaseAndInputTypeFromclientLobby();
        }else if(gameSelected == constants::gamesEnum::BLUFF){
            clientBluffPtr->setBaseAndInputTypeFromClientLobby();
        }
    }else{
        setInputStatementClientLobby();
        PF::setInputStatementHomeAccumulate();
        sati::getInstance()->setBaseAndInputType(this,
                                                 inputType::OPTIONSELECTIONINPUTLOBBY);
    }
}

void clientLobby::readMoreFailInClientSession(std::error_code ec) {
    if(ec == asio::error::eof){
        std::make_shared<home>(io)->run();
    }
}
