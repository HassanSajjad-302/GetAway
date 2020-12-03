#include <algorithm>
#include <utility>
#include "clientLobbyManager.hpp"
#include "messageTypeEnums.hpp"
#include "sati.hpp"
#include "constants.h"

clientLobbyManager::clientLobbyManager(){
    //TODO
    //DO NOT USE RESIZE AND ELSE EMPLACE BACK
    constants::initializeCards(myCards);
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
            case lobbyMessageType::GAMEFIRSTTURNSERVER:{
                int handSize = 0;
                //STEP 2;
                in.read(reinterpret_cast<char*>(&handSize), sizeof(handSize));
                assert((handSize <= ((constants::DECKSIZE / gamePlayers.size()) + 1) &&
                        handSize >= ((constants::DECKSIZE / gamePlayers.size()) - 1)) &&
                       "Unexpected Number Of Cards Received");
                for(int i=0; i < handSize; ++i){
                    deckSuit suit;
                    int cardNumber;
                    //STEP 3;
                    in.read(reinterpret_cast<char *>(&suit), sizeof(suit));
                    //STEP 4;
                    in.read(reinterpret_cast<char*>(&cardNumber), sizeof(cardNumber));
                    myCards.find(suit)->second.emplace(cardNumber);
                }

                //STAGE 2;
                for(int i=0;i<gamePlayers.size();++i){
                    int sequenceId;
                    //STEP 5; //turn sequence
                    in.read(reinterpret_cast<char*>(&sequenceId), sizeof(sequenceId));
                    assert(gamePlayers.find(sequenceId) != gamePlayers.end() && "turn SequenceId not present in gamePlayerid");
                    turnSequence.emplace_back(sequenceId);
                }

                //STAGE 3;
                int turnAlreadyDeterminedSize;
                //STEP 5;
                in.read(reinterpret_cast<char*>(&turnAlreadyDeterminedSize), sizeof(turnAlreadyDeterminedSize));
                assert(turnAlreadyDeterminedSize <= gamePlayers.size() && turnAlreadyDeterminedSize > 0 &&
                "turnAlreadyDeterminedSize not in range 0-gamePlayers.size()");
                constants::initializeCards(flushedCards);
                for(int i=0; i<turnAlreadyDeterminedSize; ++i) {
                    int turnAlreadyDeterminedId;
                    deckSuit suit;
                    int cardNumber;
                    //STEP 7;
                    in.read(reinterpret_cast<char *>(&turnAlreadyDeterminedId), sizeof(turnAlreadyDeterminedId));
                    //STEP 8;
                    in.read(reinterpret_cast<char *>(&suit), sizeof(suit));
                    //STEP 9;
                    in.read(reinterpret_cast<char *>(&cardNumber), sizeof(cardNumber));
                    roundTurns.emplace_back(turnAlreadyDeterminedId, Card(suit, cardNumber));
                    flushedCards.find(suit)->second.emplace(cardNumber);
                }
                for(int i=0; i<gamePlayers.size(); ++i){
                    int gpId;
                    int gpSize;
                    //STEP 9;
                    in.read(reinterpret_cast<char*>(&gpId), sizeof(gpId));
                    //STEP 10;
                    in.read(reinterpret_cast<char*>(&gpSize), sizeof(gpSize));
                }
                for(auto& gp: gamePlayers){
                    if(!std::any_of(roundTurns.begin(), roundTurns.end(), [&gp](std::tuple<int, Card>& tup){
                        return gp.first == std::get<0>(tup);
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
                int senderId;
                deckSuit suit;
                int cardNumber;
                //STEP 2;
                in.read(reinterpret_cast<char*>(&senderId), sizeof(senderId));
                //STEP 3;
                in.read(reinterpret_cast<char *>(&suit), sizeof(suit));
                //STEP 4;
                in.read(reinterpret_cast<char*>(&cardNumber), sizeof(cardNumber));
                managementGAMETURNSERVERReceived(senderId, Card(suit, cardNumber));
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

void clientLobbyManager::sendGAMETURNCLIENT(Card card){
    //STEP 1;
    lobbyMessageType t = lobbyMessageType::GAMETURNCLIENT;
    clientLobbySession->out.write(reinterpret_cast<char*>(&t), sizeof(t));
    //TODO
    //Check if I can send and receive card in one go.
    //STEP 2;
    clientLobbySession->out.write(reinterpret_cast<char *>(&card.suit), sizeof(card.suit));
    //STEP 2;
    clientLobbySession->out.write(reinterpret_cast<char *>(&card.cardNumber), sizeof(card.cardNumber));

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
                if(firstRound){
                    if(std::find(waitingForTurn.begin(), waitingForTurn.end(), id) == waitingForTurn.end()){
                        //don't perform turn
                        success = inputHelper(inputString, 1, 2, inputType::GAMEINT, inputType::GAMEINT,
                                              input);
                    }else{
                        success = inputHelper(inputString, 1, 3, inputType::GAMEINT, inputType::GAMEINT,
                                              input);
                    }
                }else{
                    if(senderIdExpected != id){
                        //don't perform turn
                        success = inputHelper(inputString, 1, 2, inputType::GAMEINT, inputType::GAMEINT,
                                              input);
                    }else{
                        success = inputHelper(inputString, 1, 3, inputType::GAMEINT, inputType::GAMEINT,
                                              input);
                    }
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
                        sati::getInstance()->setInputStatement3GameAccumulatePrint(turnAbleCards);
                        setInputType(inputType::GAMEPERFORMTURN);
                    }
                }
                break;
            }
            case inputType::GAMESTRING:
            {
                if(!inputString.empty()){
                    chatMessageString = std::move(inputString);
                    chatMessageInt = id;
                    sendCHATMESSAGE();
                }
                setInputTypeGameInt();
                break;
            }
            case inputType::GAMEPERFORMTURN:{
                if(inputString.empty()){
                    setInputTypeGameInt();
                }else{
                    int input;
                    if(inputHelper(inputString, 1, turnAbleCards.size(),
                                   inputType::GAMEINT, inputType::GAMEINT,input)){
                        sendGAMETURNCLIENT(turnAbleCards[input - 1]);
                        managementGAMETURNSERVERReceived(id, turnAbleCards[input - 1]);
                    }
                }
                break;
            }
        }
    }
    else{
        std::cout<<"InputReceived Type not same as Input Received Type Expected"<<std::endl;
    }
}

void clientLobbyManager::setInputTypeGameInt(){
    if(firstRound){
        if(std::find(waitingForTurn.begin(), waitingForTurn.end(), id) == waitingForTurn.end()){
            sati::getInstance()->setInputStatementHomeTwoInputGameAccumulatePrint();
        }else{
            sati::getInstance()->setInputStatementHomeThreeInputGameAccumulatePrint();
        }
    }else{
        if(senderIdExpected != id){
            sati::getInstance()->setInputStatementHomeTwoInputGameAccumulatePrint();
        }else{
            sati::getInstance()->setInputStatementHomeThreeInputGameAccumulatePrint();
        }
    }
    setInputType(inputType::GAMEINT);
}

void clientLobbyManager::managementGAMETURNSERVERReceived(int senderId, Card cardReceived) {
    if(firstRound && std::find(
            waitingForTurn.begin(), waitingForTurn.end(), senderId) != waitingForTurn.end()) {
        flushedCards.find(cardReceived.suit)->second.emplace(cardReceived.cardNumber);
        roundTurns.emplace_back(senderId, cardReceived);
        numberOfCards.find(senderId)->second -= 1;
        sati::getInstance()->setRoundTurnsGamePrint(roundTurns, gamePlayers);
        if (roundTurns.size() == turnSequence.size()) {
            for (auto t: roundTurns) {
                if (std::get<1>(t).suit == deckSuit::SPADE && std::get<1>(t).cardNumber == 0) {
                    //This is ace of spade
                    senderIdExpected = std::get<0>(t);
                    firstRound = false;
                    if(id == senderIdExpected){
                        assignToTurnAbleCards();
                    }
                    break;
                }
            }
            roundTurns.clear();
        }
        setInputTypeGameInt();
        sati::getInstance()->setCardsGameAccumulatePrint(myCards);
    }else if(!firstRound && senderId != senderIdExpected){
        //There is no check whether card number is in range 0-51
        //client is quite dumb. It will have to believe in what it receives.
        if(roundTurns.empty()){
            suitOfTheRound = cardReceived.suit;
            helperFirstTurnAndMiddleTurn(senderId, cardReceived);
        }else if(cardReceived.suit != suitOfTheRound){
            int rk = roundKing();
            numberOfCards.find(rk)->second += turnSequence.size();
            roundTurns.emplace_back(senderId, cardReceived);
            helperLastTurnAndThullaTurn(rk, true);
        }else if(roundTurns.size() == turnSequence.size() - 1){
            for(auto c: roundTurns){
                flushedCards.find(std::get<1>(c).suit)->second.emplace(std::get<1>(c).cardNumber);
            }
            roundTurns.emplace_back(senderId, cardReceived);
            helperLastTurnAndThullaTurn(roundKing(), false);
        }else{
            helperFirstTurnAndMiddleTurn(senderId, cardReceived);
        }
    }else{
        std::cout << "No Message Was Expected From This SenderId"<<std::endl;
    }
}

void clientLobbyManager::helperFirstTurnAndMiddleTurn(int senderId, Card card) {
    roundTurns.emplace_back(senderId, card);
    senderIdExpected = nextInTurnSequence(senderId);
    if(senderIdExpected == id){
        assignToTurnAbleCards(suitOfTheRound);
        sati::getInstance()->setInputStatementHomeThreeInputGamePrint();
    }
    sati::getInstance()->setRoundTurnsGameAccumulatePrint(roundTurns, gamePlayers);
}

void clientLobbyManager::helperLastTurnAndThullaTurn(int nextTurnId, bool thullaTurn) {
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
        if(thullaTurn){
            //badranga
            assignToTurnAbleCards();
        }else{
            assignToTurnAbleCards(suitOfTheRound);
        }
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

int clientLobbyManager::roundKing() {
    int highestCardHolderId = std::get<0>(*roundTurns.begin());
    int highestCardNumber = std::get<1>(*roundTurns.begin()).cardNumber;
    for(auto turns: roundTurns){
        int cardNumber = std::get<1>(turns).cardNumber;
        assert((cardNumber >= 0 && cardNumber < constants::SUITSIZE) && "Card-Number not in range");
        if(cardNumber == 0 ){
            return std::get<0>(turns);
            break;
        }else{
            if(cardNumber > highestCardNumber){
                highestCardNumber = cardNumber;
                highestCardHolderId = std::get<0>(turns);
            }
        }
    }
    return highestCardHolderId;
}

void clientLobbyManager::managementGAMEFIRSTTURNSERVERReceived() {
    debug = &waitingForTurn[0];
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
        if(myCards.find(deckSuit::SPADE)->second.empty()){
            //badranga
            assignToTurnAbleCards();
        }else{
            assignToTurnAbleCards(deckSuit::SPADE);
        }
        sati::getInstance()->setInputStatementHomeThreeInputGameAccumulatePrint();
        senderIdExpected = id;
    }
    inputTypeExpected = inputType::GAMEINT;
    if(waitingForTurn.empty()){
        firstRound = false;
    }
    else{
        firstRound = true;
    }
}

void clientLobbyManager::setInputType(inputType inputType) {
    inputTypeExpected = inputType;
    sati::getInstance()->setInputType(inputType);
}

void clientLobbyManager::assignToTurnAbleCards(){
    turnAbleCards.clear();
    for(auto& cardPair: myCards){
        for(auto c: cardPair.second){
            turnAbleCards.emplace_back(static_cast<deckSuit>(cardPair.first), c);
        }
    }
}

void clientLobbyManager::assignToTurnAbleCards(deckSuit suit) {
    turnAbleCards.clear();
    for(auto c: myCards.find(suit)->second){
        turnAbleCards.emplace_back(suit, c);
    }
}