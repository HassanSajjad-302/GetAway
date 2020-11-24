#include <algorithm>
#include <utility>
#include "clientLobbyManager.hpp"
#include "messageTypeEnums.hpp"
#include "sati.hpp"
clientLobbyManager::clientLobbyManager(){
    //TODO
    //DO NOT USE RESIZE AND ELSE EMPLACE BACK
    std::set<int> s;
    for(int i=0;i<4;++i){
        myCards.emplace(i, s);
    }
}

clientLobbyManager::~clientLobbyManager() = default;

void clientLobbyManager::join(std::shared_ptr<session<clientLobbyManager>> clientLobbySession_) {
    clientLobbySession = std::move(clientLobbySession_);
    messageTypeExpected.clear();
    messageTypeExpected.emplace_back(lobbyMessageType::SELFANDSTATE);
    clientLobbySession->receiveMessage();
}

std::istream &operator>>(std::istream &in, clientLobbyManager &manager) {
    lobbyMessageType messageTypeReceived;
    in.read(reinterpret_cast<char*>(&messageTypeReceived), sizeof(lobbyMessageType));
    if(std::find(manager.messageTypeExpected.begin(), manager.messageTypeExpected.end(), messageTypeReceived) == manager.messageTypeExpected.end()){
        std::cout<<"Unexpected Packet Type Received in class clientLobbyManager"<<std::endl;
    }else{
        switch(messageTypeReceived){
            //STEP 1;
            case lobbyMessageType::SELFANDSTATE: {
                int id;
                //STEP 2
                in.read(reinterpret_cast<char*>(&id),sizeof(id));
                manager.id = id;
                //TODO
                char arr[61]; //This constant will be fed from somewhere else but one is added.
                //STEP 3
                in.getline(arr, 61);
                std::string playerName(arr);
                //TODO
                //Action on if a new playerName is assigned.
                manager.playerName = std::move(playerName);

                int size;
                //STEP 4
                in.read(reinterpret_cast<char*>(&size), sizeof(size));
                for(int i=0; i<size; ++i){
                    int playersId = 0;
                    //STEP 5
                    in.read(reinterpret_cast<char*>(&playersId), sizeof(playersId));
                    //STEP 6
                    in.getline(arr, 61);
                    playerName = std::string(arr);
                    manager.gamePlayers.emplace(playersId, playerName);
                }
                manager.managementLobbyReceived();
                break;
            }
                //STEP 1;
            case lobbyMessageType::PLAYERJOINED:{
                int playerId = 0;
                //STEP 2;
                in.read(reinterpret_cast<char*>(&playerId), sizeof(playerId));
                char arr[61];
                //STEP 3;
                in.getline(arr, 61);
                std::string playerName(arr);
                manager.gamePlayers.emplace(playerId, playerName);
                manager.managementLobbyReceived();
                manager.clientLobbySession->receiveMessage();
                break;
            }
            case lobbyMessageType::PLAYERLEFT:{
                int playerId = 0;
                in.read(reinterpret_cast<char*>(&playerId), sizeof(playerId));
                manager.gamePlayers.erase(manager.gamePlayers.find(playerId));
                manager.managementLobbyReceived();
                manager.clientLobbySession->receiveMessage();
                break;
            }
                //STEP 1;
            case lobbyMessageType::CHATMESSAGEID: {
                //STEP 2;
                in.read(reinterpret_cast<char*>(&manager.chatMessageInt), sizeof(manager.chatMessageInt));
                assert(manager.chatMessageInt != manager.id);
                int arrSize = (manager.receivedPacketSize - 8) + 1; //4 for packety type enum and 4 for the id and 1 for getline
                char arr[arrSize];
                //STEP 3;
                in.getline(arr, manager.receivedPacketSize - 8);
                manager.chatMessageString = std::string(arr);
                sati::getInstance()->addMessageAccumulatePrint(manager.gamePlayers.find(manager.chatMessageInt)->second,
                                                               manager.chatMessageString);
                manager.clientLobbySession->receiveMessage();
                break;
            }
                //STEP 1;
            case lobbyMessageType::GAMEFIRSTTURNSERVER
                :{
                int handSize = 0;
                //STEP 2;
                in.read(reinterpret_cast<char*>(&handSize), sizeof(handSize));
                assert(handSize <= ((52 / manager.gamePlayers.size()) + 1) && "Unexpected Number Of Cards Received");
                for(int i=0; i < handSize; ++i){
                    //STEP 3;
                    int cardNumber;
                    in.read(reinterpret_cast<char*>(&cardNumber), sizeof(cardNumber));
                    manager.myCards.find(cardNumber/13)->second.emplace(cardNumber%13);
                }
                for(int i=0;i<manager.gamePlayers.size();++i){
                    int sequenceId;
                    //STEP 4; //turn sequence
                    in.read(reinterpret_cast<char*>(&sequenceId), sizeof(sequenceId));
                    assert(manager.gamePlayers.find(sequenceId) != manager.gamePlayers.end() && "turn SequenceId not present in manager.gamePlayerid");
                    manager.turnSequence.emplace_back(sequenceId);
                }
                int turnAlreadyDeterminedSize;
                //STEP 5;
                in.read(reinterpret_cast<char*>(&turnAlreadyDeterminedSize), sizeof(turnAlreadyDeterminedSize));
                assert(turnAlreadyDeterminedSize <= manager.gamePlayers.size() && turnAlreadyDeterminedSize > 0 &&
                "turnAlreadyDeterminedSize not in range 0-manager.gamePlayers.size()");
                std::vector<std::tuple<int, int>> turnAlreadyDetermined;
                for(int i=0; i<turnAlreadyDeterminedSize; ++i){
                    int id;
                    int cardNumber;
                    //STEP 6;
                    in.read(reinterpret_cast<char*>(&id), sizeof(id));
                    //STEP 7;
                    in.read(reinterpret_cast<char*>(&cardNumber), sizeof(cardNumber));
                    turnAlreadyDetermined.emplace_back(id, cardNumber);
                }
                for(auto& gp: manager.gamePlayers){
                    if(!std::any_of(turnAlreadyDetermined.begin(), turnAlreadyDetermined.end(), [&gp](std::tuple<int, int>& tup){
                        return gp.first == std::get<0>(tup);
                    })){
                        manager.waitingForTurn.emplace_back(gp.first);
                    }
                }
                assert(manager.waitingForTurn.size() == manager.gamePlayers.size() - turnAlreadyDeterminedSize &&
                "waitingForTurn vector size mismatch error operator>> clientLobbyManager.cpp");
                manager.managementGAMEFIRSTTURNSERVERReceived(std::move(turnAlreadyDetermined));
                break;
            }
        }
    }
    return in;
}

std::ostream &operator<<(std::ostream &out, clientLobbyManager &manager) {
    //STEP 1;
    lobbyMessageType t = lobbyMessageType::CHATMESSAGE;
    out.write(reinterpret_cast<char*>(&t), sizeof(t));
    //STEP 2;
    out.write(reinterpret_cast<char *>(&manager.id), sizeof(manager.id));
    //STEP 3;
    out << manager.chatMessageString << std::endl;
    return out;
}

void clientLobbyManager::uselessWriteFunction(){
    sati::getInstance()->addMessageAccumulatePrint(gamePlayers.find(chatMessageInt)->second, chatMessageString);
}


void clientLobbyManager::managementLobbyReceived(){
    messageTypeExpected.clear();
    messageTypeExpected.emplace_back(lobbyMessageType::CHATMESSAGEID);
    messageTypeExpected.emplace_back(lobbyMessageType::PLAYERJOINED);
    messageTypeExpected.emplace_back(lobbyMessageType::PLAYERLEFT);
    messageTypeExpected.emplace_back(lobbyMessageType::GAMEFIRSTTURNSERVER);
    sati::getInstance()->addOrRemovePlayerLobbyPrint(gamePlayers);
    sati::getInstance()->setInputStatementHomeLobbyAccumulatePrint();
    setInputType(inputType::LOBBYINT);
    sati::getInstance()->setBase(this);
    clientLobbySession->receiveMessage();
}


void clientLobbyManager::exitGame(){
    clientLobbySession->sock.shutdown(net::socket_base::shutdown_both);
    clientLobbySession->sock.close();
    clientLobbySession.reset();
}

bool clientLobbyManager::inputHelper(const std::string& inputString, int lower, int upper, inputType notInRange_,
                                     inputType invalidInput_, int& input_){
    try{
        int num = std::stoi(inputString);
        if(num>=lower && num<=upper){
            input_ = num;
            return true;
        }else{
            sati::getInstance()->accumulatePrint();
            sati::getInstance()->setInputType(notInRange_);
            std::cout<<"Please enter integer in range \r"<<std::endl;            return false;
        }
    }
    catch (std::invalid_argument& e) {
        sati::getInstance()->accumulatePrint();
        sati::getInstance()->setInputType(invalidInput_);
        std::cout<<"Invalid Input. \r"<<std::endl;        return false;
    }
}

//Before refactor, lines of this function = 123;
void clientLobbyManager::input(std::string inputString, inputType inputReceivedType) {
    if(inputReceivedType == inputTypeExpected){
        switch (inputReceivedType) {
            case inputType::LOBBYINT:
            {
                int input;
                if(inputHelper(inputString, 1, 2, inputType::LOBBYINT, inputType::LOBBYINT,
                               input)){
                    if(input == 1){
                        sati::getInstance()->setInputStatementMessageAccumulatePrint();
                        setInputType(inputType::LOBBYSTRING);
                    }
                    if(input== 2){
                        exitGame();
                    }
                }
                break;
            }
            case inputType::LOBBYSTRING:
            {
                if(inputString.empty()){
                    sati::getInstance()->setInputStatementHomeLobbyAccumulatePrint();
                    setInputType(inputType::LOBBYINT);
                }else{
                    chatMessageString = std::move(inputString);
                    chatMessageInt = id;
                    clientLobbySession->sendMessage(&clientLobbyManager::uselessWriteFunction);
                    sati::getInstance()->setInputStatementHomeLobbyAccumulatePrint();
                    setInputType(inputType::LOBBYINT);
                }
                break;
            }
            case inputType::GAMEINT:
            {
                int input;
                bool success;
                if(std::find(waitingForTurn.begin(), waitingForTurn.end(), id) == waitingForTurn.end()){
                    //don't perform turn
                    success = inputHelper(inputString, 1, 2, inputType::GAMEINT, inputType::GAMEINT,
                                          input);
                }else{
                    success = inputHelper(inputString, 1, 3, inputType::GAMEINT, inputType::GAMEINT,
                                          input);
                }
                if(success){
                    if(input == 1){
                        sati::getInstance()->setInputStatementMessageAccumulatePrint();
                        setInputType(inputType::GAMESTRING);
                    }
                    if(input == 2){
                        exitGame();
                    }
                    if(input == 3){
                        //perform turn
                        if(!badranga){
                            sati::getInstance()->setInputStatement3GameAccumulatePrint(myCards.find((int)suitOfTheRound)->second,
                                                                                       (int)suitOfTheRound);
                        }
                        else{
                            sati::getInstance()->setInputStatement3GameAccumulatePrint(myCards);
                        }
                        setInputType(inputType::GAMEPERFORMTURN);
                    }
                }
                break;
            }
            case inputType::GAMESTRING:
            {
                if(inputString.empty()){
                    if(std::find(waitingForTurn.begin(), waitingForTurn.end(), id) == waitingForTurn.end()){
                        sati::getInstance()->setInputStatementHomeTwoInputGameAccumulatePrint();
                    }else{
                        sati::getInstance()->setInputStatementHomeThreeInputGameAccumulatePrint();
                    }
                    setInputType(inputType::GAMEINT);
                }else{
                    chatMessageString = std::move(inputString);
                    chatMessageInt = id;
                    clientLobbySession->sendMessage(&clientLobbyManager::uselessWriteFunction);
                    if(std::find(waitingForTurn.begin(), waitingForTurn.end(), id) == waitingForTurn.end()){
                        sati::getInstance()->setInputStatementHomeTwoInputGameAccumulatePrint();
                    }else{
                        sati::getInstance()->setInputStatementHomeThreeInputGameAccumulatePrint();
                    }
                    setInputType(inputType::GAMEINT);
                }
                break;
            }
            case inputType::GAMEPERFORMTURN:{

                break;
            }
        }
    }
    else{
        std::cout<<"InputReceived Type not same as Input Received Type Expected"<<std::endl;
    }
}

void clientLobbyManager::managementGAMEFIRSTTURNSERVERReceived(std::vector<std::tuple<int, int>> turnAlreadyDetermined_) {
    messageTypeExpected.clear();
    messageTypeExpected.emplace_back(lobbyMessageType::CHATMESSAGEID);
    messageTypeExpected.emplace_back(lobbyMessageType::GAMETURNSERVER);
    clientLobbySession->receiveMessage();

    //Printing Starts
    sati::getInstance()->setTurnSequenceGamePrint(gamePlayers, turnSequence);

    for(auto& tu: turnAlreadyDetermined_){
        assert((std::get<1>(tu)/13) == (int)deckSuit::SPADE && "Card Receved Not In Spade");
        sati::getInstance()->addTurnGamePrint(gamePlayers.find(std::get<0>(tu))->second, std::get<1>(tu));
    }

    sati::getInstance()->setWaitingForTurnGamePrint(waitingForTurn, gamePlayers); //waiting for turn
    sati::getInstance()->setCardsGamePrint(myCards); //cards


    sati::getInstance()->setReceiveInputTypeAndGameStarted(inputType::GAMEINT,true);
    //set input statement and print all this
    if(std::find(waitingForTurn.begin(), waitingForTurn.end(), id) == waitingForTurn.end()){
        //don't perform turn
        sati::getInstance()->setInputStatementHomeTwoInputGameAccumulatePrint();
    }else{
        sati::getInstance()->setInputStatementHomeThreeInputGameAccumulatePrint();
    }
    inputTypeExpected = inputType::GAMEINT;
}

void clientLobbyManager::setInputType(inputType inputType) {
    inputTypeExpected = inputType;
    sati::getInstance()->setInputType(inputType);
}