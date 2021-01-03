
#include <sati.hpp>
#include "clientRoomManager.hpp"
#include "messageTypeEnums.hpp"
#include "clientHome.hpp"

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
        switch(innerMessageType) {
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
                resourceStrings::print("Unexpected Packet Type Received in class serverRoomManager\r\n");
                break;
            }
        }
    }
    else{
        resourceStrings::print("Unexpected Packet Type Received in class serverRoomManager\r\n");
    }
}

void clientRoomManager::SELFANDSTATEReceived(){
    messageTypeExpected.clear();
    messageTypeExpected.emplace_back(messageType::CHATMESSAGEID);
    messageTypeExpected.emplace_back(messageType::PLAYERJOINED);
    messageTypeExpected.emplace_back(messageType::PLAYERLEFT);
    messageTypeExpected.emplace_back(messageType::GAMEFIRSTTURNSERVER);
    setInputType(inputType::LOBBYINT);
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
            case inputType::LOBBYINT:
            {
                int input;
                if(constants::inputHelper(inputString, 1, 3, inputType::LOBBYINT, inputType::LOBBYINT,
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
            case inputType::MESSAGESTRING:
            {

                break;
            }
            case inputType::GAMEINT:
            {
                int input;
                bool success;
                if(firstRound){
                    if(std::find(waitingForTurn.begin(), waitingForTurn.end(), id) == waitingForTurn.end()){
                        //don't perform turn
                        success = constants::inputHelper(inputString, 1, 3, inputType::GAMEINT, inputType::GAMEINT,
                                                         input);
                    }else{
                        success = constants::inputHelper(inputString, 1, 4, inputType::GAMEINT, inputType::GAMEINT,
                                                         input);
                    }
                }else{
                    if(turnPlayerIdExpected != id){
                        //don't perform turn
                        success = constants::inputHelper(inputString, 1, 3, inputType::GAMEINT, inputType::GAMEINT,
                                                         input);
                    }else{
                        success = constants::inputHelper(inputString, 1, 4, inputType::GAMEINT, inputType::GAMEINT,
                                                         input);
                    }
                }

                if(success){
                    if(input == 1){
                        messagePF::setInputStatementAccumulate();
                        setInputType(inputType::MESSAGESTRING);
                    }else if(input == 2){
                        exitApplication();
                        std::make_shared<clientHome>(clientHome(io))->run();
                    }else if(input == 3){
                        exitApplication();
                    }else{
                        //perform turn
                        gamePF::setInputStatementHome3R3Accumulate(turnAbleCards);
                        setInputType(inputType::GAMEPERFORMTURN);
                    }
                }
                break;
            }
            case inputType::GAMEPERFORMTURN:{
                if(inputString.empty()){
                    setInputTypeGameInt();
                }else{
                    int input;
                    if(constants::inputHelper(inputString, 1, turnAbleCards.size(),
                                              inputType::GAMEPERFORMTURN, inputType::GAMEPERFORMTURN,input)){
                        sendGAMETURNCLIENT(turnAbleCards[input - 1]);
                        Turn(id, turnAbleCards[input - 1], whoTurned::CLIENT);
                    }else{

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
    clientLobbySession->sock.shutdown(asio::socket_base::shutdown_both);
    clientLobbySession->sock.close();
    clientLobbySession.reset();
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