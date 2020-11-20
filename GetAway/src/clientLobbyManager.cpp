#include <algorithm>
#include <utility>
#include "clientLobbyManager.hpp"
#include "messageTypeEnums.hpp"
#include "sati.hpp"
clientLobbyManager::clientLobbyManager(){
    messageTypeExpected.resize((int)lobbyMessageType::ENUMSIZE);
    messageTypeExpected[0] = lobbyMessageType::SELFANDSTATE;
    spdlog::info("ClientLobbyManager Constructor Called");
}

void clientLobbyManager::join(std::shared_ptr<session<clientLobbyManager>> clientLobbySession_) {
    clientLobbySession = std::move(clientLobbySession_);
    messageTypeExpected.clear();
    messageTypeExpected[0] = lobbyMessageType::SELFANDSTATE;
    clientLobbySession->receiveMessage();
    managementNextAction();
}

std::istream &operator>>(std::istream &in, clientLobbyManager &manager) {
    lobbyMessageType messageTypeReceived;
    in.read(reinterpret_cast<char*>(&messageTypeReceived), sizeof(lobbyMessageType));
    if(std::find(manager.messageTypeExpected.begin(), manager.messageTypeExpected.end(), messageTypeReceived) != manager.messageTypeExpected.end()){
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
                manager.messageTypeExpected[0] = lobbyMessageType::CHATMESSAGEID;
                manager.messageTypeExpected[1] = lobbyMessageType::PLAYERJOINED;
                manager.messageTypeExpected[2] = lobbyMessageType::PLAYERLEFT;
                manager.managementLobbyReceived();
                manager.clientLobbySession->receiveMessage();

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
                sati::getInstance()->messageBufferAppend(manager.gamePlayers.find(manager.chatMessageInt)->second
                                                         + ": " +  manager.chatMessageString + "\r\n");
                manager.clientLobbySession->receiveMessage();
                break;
            }
                //STEP 1;
            case lobbyMessageType::GAMEFIRSTTURNSERVER
                :{
                int numOfCards = 0;
                //STEP 2;
                in.read(reinterpret_cast<char*>(&numOfCards), sizeof(numOfCards));
                assert(numOfCards<=((52/manager.gamePlayers.size()) + 1) && "Unexpected Number Of Cards Received");
                for(int i=0; i<numOfCards; ++i){
                    //STEP 3;
                    int cardNumber;
                    in.read(reinterpret_cast<char*>(&cardNumber), sizeof(cardNumber));
                    manager.myCards.push_back(cardNumber);
                }
                int turnAlreadyDeterminedSize;
                //STEP 4;
                in.read(reinterpret_cast<char*>(&turnAlreadyDeterminedSize), sizeof(turnAlreadyDeterminedSize));
                std::vector<std::tuple<int, int>> turnAlreadyDetermined;
                for(int i=0; i<turnAlreadyDeterminedSize; ++i){
                    int id;
                    int cardNumber;
                    //STEP 5;
                    in.read(reinterpret_cast<char*>(&id), sizeof(id));
                    //STEP 6;
                    in.read(reinterpret_cast<char*>(&cardNumber), sizeof(cardNumber));
                    turnAlreadyDetermined.emplace_back(id, cardNumber);
                }
                manager.managementGAMEFIRSTTURNSERVERReceived();
                manager.messageTypeExpected[0] = lobbyMessageType::CHATMESSAGE;
                manager.messageTypeExpected[1] = lobbyMessageType::GAMETURNSERVER;
                manager.clientLobbySession->receiveMessage();
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
    sati::getInstance()->messageBufferAppend(gamePlayers.find(chatMessageInt)->second + ": " +  chatMessageString + "\r\n");
}


void clientLobbyManager::managementLobbyReceived(){
    std::string roundBufferBefore = "Players in Lobby Are\r\n";
    for(auto& player: gamePlayers) {
        roundBufferBefore += player.second;
        roundBufferBefore += "\r\n";
    }
    roundBufferBefore += "\r\n";
    sati::getInstance()->roundBufferBeforeChanged(roundBufferBefore);
}

void clientLobbyManager::managementNextAction(){
    std::string toPrint = "1)Send Message 2)Exit\r\n\n";
    sati::getInstance()->inputStatementBufferChanged(toPrint, false);
    setInputType(inputType::LOBBYINT);
    sati::getInstance()->setBase(this);
}


void clientLobbyManager::exitGame(){
    clientLobbySession->sock.shutdown(net::socket_base::shutdown_both);
    clientLobbySession->sock.close();
    clientLobbySession.reset();
}

void clientLobbyManager::input(std::string inputString, inputType inputReceivedType) {
    if(inputReceivedType == inputTypeExpected){
        switch (inputReceivedType) {
            case inputType::LOBBYINT:
            {
                try{
                    int num = std::stoi(inputString);
                    if(num>=1 && num<=2){
                        if(num == 1){
                            std::string toPrint = "Please Type The Message \r\n\n";
                            sati::getInstance()->inputStatementBufferChanged(toPrint, false);
                            setInputType(inputType::LOBBYSTRING);
                        }
                        if(num == 2){
                            sati::getInstance()->inputStatementBufferChanged("Exiting\r\n",true);
                            exitGame();
                        }
                    }else{
                        sati::getInstance()->accumulateBuffersAndPrintWithLock();
                        sati::getInstance()->setInputType(inputType::LOBBYINT);
                        std::cout<<"Please enter integer in range \r"<<std::endl;
                    }
                }
                catch (std::invalid_argument& e) {
                    sati::getInstance()->accumulateBuffersAndPrintWithLock();
                    sati::getInstance()->setInputType(inputType::LOBBYINT);
                    std::cout<<"Invalid Input. \r"<<std::endl;
                }
                break;
            }
            case inputType::LOBBYSTRING:
            {
                //TODO
                //if received string is empty, move back.
                chatMessageString = std::move(inputString);

                //TODO
                //This is an error if before the message write finishes, player receives the message
                chatMessageInt = id;
                clientLobbySession->sendMessage(&clientLobbyManager::uselessWriteFunction);
                std::string toPrint = "1)Send Message 2)Exit\r\n\n";
                sati::getInstance()->inputStatementBufferChanged(toPrint, false);
                setInputType(inputType::LOBBYINT);
                break;
            }
            case inputType::GAMEINT:
            {
                break;
            }
            case inputType::GAMESTRING:
            {
                break;
            }
        }
    }
    else{
        std::cout<<"InputReceived Type not same as Input Received Type Expected"<<std::endl;
    }
}

clientLobbyManager::~clientLobbyManager() {
    sati::getInstance()->printExitMessage("clientLobbyManagerDestructor Called");
    spdlog::info("ClientLobbyManager Destructor Called");
}

void clientLobbyManager::managementGAMEFIRSTTURNSERVERReceived() {
    std::string toPrint = "1)Send Message 2)Exit 3)Perform Turn\r\n\n";
    sati::getInstance()->inputStatementBufferChanged(toPrint, false);
    //display cards received
    //ask user for cards input
    //display timer.
}

void clientLobbyManager::setInputType(inputType inputType) {
    inputTypeExpected = inputType;
    sati::getInstance()->setInputType(inputType);
}


