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

void clientLobbyManager::packetReceivedFromNetwork(std::istream &in, int receivedPacketSize) {

    lobbyMessageType messageTypeReceived;
    in.read(reinterpret_cast<char*>(&messageTypeReceived), sizeof(lobbyMessageType));
    if(std::find(messageTypeExpected.begin(), messageTypeExpected.end(), messageTypeReceived) == messageTypeExpected.end()){
        std::cout<<"Unexpected Packet Type Received in class clientLobbyManager"<<std::endl;
    }else{
        switch(messageTypeReceived){
            //STEP 1;
            case lobbyMessageType::SELFANDSTATE: {
                //STEP 2
                in.read(reinterpret_cast<char*>(&id),sizeof(id));
                //TODO
                char arr[61]; //This constant will be fed from somewhere else but one is added.
                //STEP 3
                in.getline(arr, 61);
                //TODO
                //Action on if a new playerName is assigned.
                playerName = std::string(arr);

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
                    gamePlayers.emplace(playersId, playerName);
                }
                managementLobbyReceived();
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
                gamePlayers.emplace(playerId, std::string(arr));
                managementLobbyReceived();
                break;
            }
            case lobbyMessageType::PLAYERLEFT:{
                int playerId = 0;
                in.read(reinterpret_cast<char*>(&playerId), sizeof(playerId));
                gamePlayers.erase(gamePlayers.find(playerId));
                managementLobbyReceived();
                break;
            }
                //STEP 1;
            case lobbyMessageType::CHATMESSAGEID: {
                //STEP 2;
                in.read(reinterpret_cast<char*>(&chatMessageInt), sizeof(chatMessageInt));
                assert(chatMessageInt != id);
                int arrSize = (receivedPacketSize - 8) + 1; //4 for packety type enum and 4 for the id and 1 for getline
                char arr[arrSize];
                //STEP 3;
                in.getline(arr, receivedPacketSize - 8);
                chatMessageString = std::string(arr);
                sati::getInstance()->addMessageAccumulatePrint(gamePlayers.find(chatMessageInt)->second,
                                                               chatMessageString);
                break;
            }
                //STEP 1;
            case lobbyMessageType::GAMEFIRSTTURNSERVER
                :{
                int handSize = 0;
                //STEP 2;
                in.read(reinterpret_cast<char*>(&handSize), sizeof(handSize));
                assert(handSize <= ((52 / gamePlayers.size()) + 1) && "Unexpected Number Of Cards Received");
                for(int i=0; i < handSize; ++i){
                    int cardNumber;
                    //STEP 3;
                    in.read(reinterpret_cast<char*>(&cardNumber), sizeof(cardNumber));
                    myCards.find(cardNumber/13)->second.emplace(cardNumber%13);
                }
                for(int i=0;i<gamePlayers.size();++i){
                    int sequenceId;
                    //STEP 4; //turn sequence
                    in.read(reinterpret_cast<char*>(&sequenceId), sizeof(sequenceId));
                    assert(gamePlayers.find(sequenceId) != gamePlayers.end() && "turn SequenceId not present in gamePlayerid");
                    turnSequence.emplace_back(sequenceId);
                }
                int turnAlreadyDeterminedSize;
                //STEP 5;
                in.read(reinterpret_cast<char*>(&turnAlreadyDeterminedSize), sizeof(turnAlreadyDeterminedSize));
                assert(turnAlreadyDeterminedSize <= gamePlayers.size() && turnAlreadyDeterminedSize > 0 &&
                "turnAlreadyDeterminedSize not in range 0-gamePlayers.size()");
                for(int i=0; i<turnAlreadyDeterminedSize; ++i){
                    int turnAlreadyDeterminedId;
                    int cardNumber;
                    //STEP 6;
                    in.read(reinterpret_cast<char*>(&turnAlreadyDeterminedId), sizeof(turnAlreadyDeterminedId));
                    //STEP 7;
                    in.read(reinterpret_cast<char*>(&cardNumber), sizeof(cardNumber));
                    flushedCards.emplace_back(cardNumber);
                    roundTurns.emplace_back(cardNumber, turnAlreadyDeterminedId);
                }
                for(int i=0; i<gamePlayers.size(); ++i){
                    int gpId;
                    int gpSize;
                    //STEP 8;
                    in.read(reinterpret_cast<char*>(&gpId), sizeof(gpId));
                    //STEP 9;
                    in.read(reinterpret_cast<char*>(&gpSize), sizeof(gpSize));
                }
                for(auto& gp: gamePlayers){
                    if(!std::any_of(roundTurns.begin(), roundTurns.end(), [&gp](std::tuple<int, int>& tup){
                        return gp.first == std::get<1>(tup);
                    })){
                        waitingForTurn.emplace_back(gp.first);
                    }
                }
                assert(waitingForTurn.size() == gamePlayers.size() - turnAlreadyDeterminedSize &&
                "waitingForTurn vector size mismatch error operator>> clientLobbyManager.cpp");
                managementGAMEFIRSTTURNSERVERReceived();
                break;
            }
            //STEP 1;
            case lobbyMessageType::GAMETURNSERVER:{
                //STEP 2;
                int senderId;
                in.read(reinterpret_cast<char*>(&senderId), sizeof(senderId));
                //STEP 3;
                int cardNumber;
                in.read(reinterpret_cast<char*>(&cardNumber), sizeof(cardNumber));
                managementGAMETURNSERVERReceived(senderId, cardNumber);
                break;
            }
        }
    }
    clientLobbySession->receiveMessage();
}

void clientLobbyManager::sendCHATMESSAGE(){
    std::ostream& out = clientLobbySession->out;
    //STEP 1;
    lobbyMessageType t = lobbyMessageType::CHATMESSAGE;
    out.write(reinterpret_cast<char*>(&t), sizeof(t));
    //STEP 2;
    out.write(reinterpret_cast<char *>(&id), sizeof(id));
    //STEP 3;
    out << chatMessageString << std::endl;

    clientLobbySession->sendMessage(&clientLobbyManager::uselessWriteFunctionCHATMESSAGE);
}

void clientLobbyManager::sendGAMETURNCLIENT(int excitedCardId){
    //STEP 1;
    lobbyMessageType t = lobbyMessageType::CHATMESSAGE;
    clientLobbySession->out.write(reinterpret_cast<char*>(&t), sizeof(t));
    //STEP 2;
    clientLobbySession->out.write(reinterpret_cast<char *>(&excitedCardId), sizeof(excitedCardId));

    clientLobbySession->sendMessage(&clientLobbyManager::uselessWriteFunctionGAMETURNCLIENT);
}

void clientLobbyManager::uselessWriteFunctionCHATMESSAGE(){
    sati::getInstance()->addMessageAccumulatePrint(gamePlayers.find(chatMessageInt)->second, chatMessageString);
}

void clientLobbyManager::uselessWriteFunctionGAMETURNCLIENT(){
    //sati::getInstance()->addMessageAccumulatePrint(gamePlayers.find(chatMessageInt)->second, chatMessageString);
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
            std::cout<<"Please enter integer in range \r"<<std::endl;
            return false;
        }
    }
    catch (std::invalid_argument& e) {
        sati::getInstance()->accumulatePrint();
        sati::getInstance()->setInputType(invalidInput_);
        std::cout<<"Invalid Input. \r"<<std::endl;
        return false;
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
                    sendCHATMESSAGE();
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
                    sendCHATMESSAGE();
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
                int input;

                if(inputHelper(inputString, 1, inputHelperGAMEPERFORMTURN(), inputType::GAMEINT, inputType::GAMEINT,
                               input)){
                    input -= 1; //index adjustment
                    int cardNumber = inputHelperGAMEPERFORMTURN(input);
                    sendGAMETURNCLIENT(cardNumber);

                    auto& setReference = myCards.find(cardNumber / 13)->second;
                    assert(setReference.find(cardNumber) != setReference.end() &&
                           "Trying to remove a card which player does not has");
                    setReference.erase(cardNumber);
                    roundTurns.emplace_back(cardNumber, id);
                    if(roundTurns.size() == turnSequence.size()){
                        //round has completed. determine who will have the next turn.
                        //maybe i have given badranga.
                    }
                    roundTurns.emplace_back();
                }
                break;
            }
        }
    }
    else{
        std::cout<<"InputReceived Type not same as Input Received Type Expected"<<std::endl;
    }
}

int clientLobbyManager::inputHelperGAMEPERFORMTURN(){
    int cardsCount=0;
    for(auto &s: myCards){
        cardsCount += s.second.size();
    }
    return cardsCount;
}

int clientLobbyManager::inputHelperGAMEPERFORMTURN(int index){
    int cardNumber = 0;
    for(auto &s: myCards){
        for(auto&c: s.second){
            if(cardNumber == index){
                return cardNumber;
            }
            cardNumber += 1;
        }
    }
    throw(std::logic_error("inputHelperGAMEPERFORMTURN should have returned a number"));
}

void clientLobbyManager::managementGAMETURNSERVERReceived(int senderId, int cardNumber) {
    if(firstTurn && std::find(
            waitingForTurn.begin(), waitingForTurn.end(), senderId) != waitingForTurn.end()) {
        flushedCards.emplace_back(cardNumber);
        if (roundTurns.size() == turnSequence.size()) {
            for (auto t: roundTurns) {
                if (std::get<0>(t) / 13 == 0) {
                    //This is ace of spade
                    senderIdExpected = std::get<1>(t);
                    firstTurn = false;
                    break;
                }
            }
        }
        sati::getInstance()->setRoundTurnsGameAccumulatePrint(roundTurns, gamePlayers);
    }else if(!firstTurn && senderId != senderIdExpected){
        //There is no check whether card number is in range 0-51
        //client is quite dumb. It will have to believe in what it receives.
        if(roundTurns.empty()){
            suitOfTheRound = static_cast<deckSuit>(cardNumber / 13);
            helperFirstTurnAndMiddleTurn(cardNumber, senderId);
        }else if(cardNumber / 13 != (int)suitOfTheRound){
            int rk = roundKing();
            numberOfCards.find(rk)->second += turnSequence.size();
            roundTurns.emplace_back(cardNumber, senderId);
            helperLastTurnAndThullaTurn(rk);
        }else if(roundTurns.size() == turnSequence.size() - 1){
            for(auto c: roundTurns){
                flushedCards.emplace_back(std::get<0>(c));
            }
            roundTurns.emplace_back(cardNumber, senderId);
            helperLastTurnAndThullaTurn(roundKing());
        }else{
            helperFirstTurnAndMiddleTurn(cardNumber, senderId);
        }
    }else{
        std::cout << "No Message Was Expected From This SenderId"<<std::endl;
    }
}

void clientLobbyManager::helperFirstTurnAndMiddleTurn(int cardNumber, int senderId) {
    roundTurns.emplace_back(cardNumber, senderId);
    senderIdExpected = nextInTurnSequence(senderId);
    if(senderIdExpected == id){
        sati::getInstance()->setInputStatementHomeThreeInputGamePrint();
    }
    sati::getInstance()->setRoundTurnsGameAccumulatePrint(roundTurns, gamePlayers);
}

void clientLobbyManager::helperLastTurnAndThullaTurn(int nextTurnId){
    for(auto p: numberOfCards){
        int id1 = std::get<0>(p);
        int numOfCards = std::get<1>(p);
        if(numOfCards == 0){
            if(id1 == nextTurnId){
                auto it = std::find(turnSequence.begin(), turnSequence.end(), id1);
                if(it == turnSequence.end()){
                    nextTurnId = *turnSequence.begin();
                }else{
                    nextTurnId = *(++it);
                }
            }
            turnSequence.erase(
                    std::find(turnSequence.begin(), turnSequence.end(),std::get<0>(p)));
        }
    }
    sati::getInstance()->setTurnSequenceGamePrint(gamePlayers, turnSequence);
    senderIdExpected = nextTurnId;
    if(id == senderIdExpected){
        sati::getInstance()->setInputStatementHomeThreeInputGamePrint();
    }
    sati::getInstance()->setRoundTurnsGameAccumulatePrint(roundTurns, gamePlayers);
    roundTurns.clear();
}
int clientLobbyManager::nextInTurnSequence(int currentSessionId){
    std::vector<int>::iterator it;
    for(it = turnSequence.begin(); it != turnSequence.end(); ++it){
        if(*it == currentSessionId){
            break;
        }
    }
    assert(it != turnSequence.end() && "Id of current session is not present in turnSequence");
    auto next = ++it;
    if(next != turnSequence.end()){
        return *next;
    }
    return *turnSequence.begin();
}

int clientLobbyManager::roundKing(){
    int highestCardNumber = std::get<0>(*roundTurns.begin());
    int highestCardHolderId = std::get<1>(*roundTurns.begin());
    for(auto turns: roundTurns){
        int cardNumber = std::get<0>(turns) % 13;
        if(cardNumber == 0 ){
            return std::get<1>(turns);
            break;
        }else{
            if(cardNumber > highestCardNumber){
                highestCardNumber = cardNumber;
                highestCardHolderId = std::get<1>(turns);
            }
        }
    }
    return highestCardHolderId;
}

void clientLobbyManager::managementGAMEFIRSTTURNSERVERReceived() {
    messageTypeExpected.clear();
    messageTypeExpected.emplace_back(lobbyMessageType::CHATMESSAGEID);
    messageTypeExpected.emplace_back(lobbyMessageType::GAMETURNSERVER);

    //Printing Starts
    sati::getInstance()->setTurnSequenceGamePrint(gamePlayers, turnSequence);

    sati::getInstance()->setRoundTurnsGamePrint(roundTurns, gamePlayers);

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
    gameStarted = true;
    firstTurn = true;
}

void clientLobbyManager::setInputType(inputType inputType) {
    inputTypeExpected = inputType;
    sati::getInstance()->setInputType(inputType);
}