#include <algorithm>
#include <utility>
#include "clientLobbyManager.hpp"
#include "messageTypeEnums.hpp"
#include "sati.hpp"
#include "constants.h"
#include "gamePF.hpp"
#include "messagePF.hpp"

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
                messagePF::addAccumulate(gamePlayers.find(chatMessageInt)->second,
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

                for(int i=0; i<gamePlayers.size(); ++i){
                    int gpId;
                    int gpSize;
                    //STEP 9;
                    in.read(reinterpret_cast<char*>(&gpId), sizeof(gpId));
                    //STEP 10;
                    in.read(reinterpret_cast<char*>(&gpSize), sizeof(gpSize));
                    numberOfCards.emplace(gpId, gpSize);
                }
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
                spdlog::info("GameTurnServerReceived From Id {}. Card Is {} {}",
                             senderId, deckSuitValue::displaySuitType[(int) suit], deckSuitValue::displayCards[cardNumber]);
                Turn(senderId, Card(suit, cardNumber), whoTurned::RECEIVED);
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
    spdlog::info("Seding Card Message To Server. Suit {} {}",
                 deckSuitValue::displaySuitType[(int)card.suit], deckSuitValue::displayCards[card.cardNumber]);
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
    messagePF::addAccumulate(gamePlayers.find(chatMessageInt)->second, chatMessageString);
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
    setInputType(inputType::LOBBYINT);
    sati::getInstance()->setBase(this, appState::LOBBY);
    lobbyPF::addOrRemovePlayer(gamePlayers);
    lobbyPF::setInputStatementHomeAccumulate();
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
                        messagePF::setInputStatementAccumulate();
                        setInputType(inputType::MESSAGESTRING);
                    }
                    if(input== 2){
                        exitGame();
                    }
                }
                break;
            }
            case inputType::MESSAGESTRING:
            {
                if(inputString.empty()){
                    lobbyPF::setInputStatementHomeAccumulate();
                    gameStarted ? setInputTypeGameInt() : setInputType(inputType::LOBBYINT);
                }else{
                    chatMessageString = std::move(inputString);
                    chatMessageInt = id;
                    sendCHATMESSAGE();
                    lobbyPF::setInputStatementHomeAccumulate();
                    gameStarted ? setInputTypeGameInt() : setInputType(inputType::LOBBYINT);
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
                    if(turnPlayerIdExpected != id){
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
                        messagePF::setInputStatementAccumulate();
                        setInputType(inputType::MESSAGESTRING);
                    }
                    if(input == 2){
                        exitGame();
                    }
                    if(input == 3){
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
                    if(inputHelper(inputString, 1, turnAbleCards.size(),
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
        std::cout<<"InputReceived Type not same as Input Received Type Expected"<<std::endl;
    }
}

void clientLobbyManager::setInputTypeGameInt(){
    if(firstRound){
        if(std::find(waitingForTurn.begin(), waitingForTurn.end(), id) == waitingForTurn.end()){
            gamePF::setInputStatementHome2Accumulate();
        }else{
            gamePF::setInputStatementHome3Accumulate();
        }
    }else{
        if(turnPlayerIdExpected != id){
            gamePF::setInputStatementHome2Accumulate();
        }else{
            gamePF::setInputStatementHome3Accumulate();
        }
    }
    setInputType(inputType::GAMEINT);
}
void clientLobbyManager::firstRoundTurnHelper(int playerId, Card card, whoTurned who){
    spdlog::info("firstRoundTurnHelper Called");
    waitingForTurn.erase(std::find(waitingForTurn.begin(), waitingForTurn.end(), playerId));
    if(who == whoTurned::CLIENT){
        myCards.find(card.suit)->second.erase(card.cardNumber);
        gamePF::setCards(myCards);
        setInputTypeGameInt();
    }
    flushedCards.find(card.suit)->second.emplace(card.cardNumber);
    roundTurns.emplace_back(playerId, card);
    gamePF::setRoundTurnsAccumulate(roundTurns, gamePlayers);
    numberOfCards.find(playerId)->second -= 1;

    if (roundTurns.size() == turnSequence.size()) {
        assert(waitingForTurn.empty() && "A New Round Is Starting When Old Round Turns Are Not Received Yet.");
        for (auto t: roundTurns) {
            if (std::get<1>(t).suit == deckSuit::SPADE && std::get<1>(t).cardNumber == 0) {
                //This is ace of spade
                turnPlayerIdExpected = std::get<0>(t);
                firstRound = false;
                //Here I am auto turning. So, here I have to check whether auto turn is possible and if yes then
                //auto turn but do not send it to the server and just print it to the screen.
                if(id == turnPlayerIdExpected){
                    assignToTurnAbleCards();
                    setInputTypeGameInt();
                }
                break;
            }
        }
        roundTurns.clear();
    }
}
void clientLobbyManager::Turn(int playerId, Card card, whoTurned who) {
    if(firstRound && std::find(
            waitingForTurn.begin(), waitingForTurn.end(), playerId) != waitingForTurn.end()) {
        firstRoundTurnHelper(playerId, card, who);
    }else if(!firstRound && playerId == turnPlayerIdExpected){
        //There is no check whether card number is in range 0-51
        //client is quite dumb. It will have to believe in what it receives.
        if(roundTurns.empty()){
            helperFirstTurnAndMiddleTurn(playerId, card, true, who);
        }else if(card.suit != suitOfTheRound){
            helperLastTurnAndThullaTurn(playerId, card, true, who);
        }else if(roundTurns.size() == turnSequence.size() - 1){
            helperLastTurnAndThullaTurn(playerId, card, false, who);
        }else{
            helperFirstTurnAndMiddleTurn(playerId, card, false, who);
        }
        setInputTypeGameInt();
    }else{
        spdlog::info("No Message Was Expected From This User");
    }
}

void clientLobbyManager::helperFirstTurnAndMiddleTurn(int playerId, Card card, bool firstTurn, whoTurned who) {
    spdlog::info("FirstTurnAndMiddleTurn Called. firstTurn value is {}", std::to_string(firstTurn));
    if(firstTurn){
        suitOfTheRound = card.suit;
        spdlog::info("Suit Of The Round is {}", deckSuitValue::displaySuitType[(int)suitOfTheRound]);
    }
    roundTurns.emplace_back(playerId, card);
    gamePF::setRoundTurns(roundTurns, gamePlayers);

    if(playerId == id){
        myCards.find(card.suit)->second.erase(card.cardNumber);
        gamePF::setCards(myCards);
    }


    turnPlayerIdExpected = nextInTurnSequence(playerId);
    spdlog::info("turnPlayerIdExpected is {}", turnPlayerIdExpected);
    if(turnPlayerIdExpected == id){
        if(myCards.find(card.suit)->second.empty()){
            assignToTurnAbleCards();
        }else{
            assignToTurnAbleCards(suitOfTheRound);
        }
    }
}

void clientLobbyManager::helperLastTurnAndThullaTurn(int playerId, Card card, bool thullaTurn, whoTurned who) {
    spdlog::info("helperLastTurnAndThullaTurn Called. thullaTurn is {}", thullaTurn);
    if(thullaTurn){
        turnPlayerIdExpected = roundKing();
        spdlog::info("Thulla Receival Id is {}", turnPlayerIdExpected);
        //There are two possibilities and we are interested in both. roundking may be our id or the
        //playerId may be our Id.
        assert(turnPlayerIdExpected != playerId && "Why we are performing thulla turn if we have already performed our turn in the round");
        roundTurns.emplace_back(playerId, card);
        gamePF::setRoundTurns(roundTurns, gamePlayers);

        if(playerId == id){
            myCards.find(card.suit)->second.erase(card.cardNumber);
            gamePF::setCards(myCards);
        }else if(turnPlayerIdExpected == id){
            spdlog::info("I received a thulla");
            for(auto thullaCards: roundTurns){
                myCards.find(std::get<1>(thullaCards).suit)->second.emplace(std::get<1>(thullaCards).cardNumber);
            }
            gamePF::setCards(myCards);
        }

        for(auto& rt: roundTurns){
            numberOfCards.find(std::get<0>(rt))->second -= 1;
        }
        numberOfCards.find(turnPlayerIdExpected)->second += roundTurns.size();
    }else{
        roundTurns.emplace_back(playerId, card);
        gamePF::setRoundTurns(roundTurns, gamePlayers);

        turnPlayerIdExpected = roundKing();
        if(playerId == id){
            myCards.find(card.suit)->second.erase(card.cardNumber);
            gamePF::setCards(myCards);
        }
        for(auto& rt: roundTurns){
            flushedCards.find(std::get<1>(rt).suit)->second.emplace(std::get<1>(rt).cardNumber);
            numberOfCards.find(std::get<0>(rt))->second -= 1;
        }
    }
    roundTurns.clear();

    //Anyone whose cards are ended is removed from turnSequence.
    for(auto p: numberOfCards){
        int id1 = std::get<0>(p);
        int numOfCards = std::get<1>(p);
        if(numOfCards == 0){
            spdlog::info("numberOfCards of id {} are 0", id1);
            if(id1 == turnPlayerIdExpected){
                auto it = std::find(turnSequence.begin(), turnSequence.end(), id1);
                if(it == turnSequence.end()){
                    turnPlayerIdExpected = *turnSequence.begin();
                }else{
                    turnPlayerIdExpected = *(++it);
                }
            }
            turnSequence.erase(
                    std::find(turnSequence.begin(), turnSequence.end(),std::get<0>(p)));
        }
    }
    gamePF::setTurnSequence(gamePlayers, turnSequence);

    if(turnSequence.empty()){
        //game drawn
        //move back to lobby
    }else if(turnSequence.size() == 1){
        int loosingId = *turnSequence.begin();
        //move back to lobby
    }else{
        //game continues
    }

    if(turnPlayerIdExpected == id){
        assignToTurnAbleCards();
    }
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
            int id3 = std::get<0>(turns);
            return id3;
        }else{
            if(cardNumber > highestCardNumber){
                highestCardNumber = cardNumber;
                highestCardHolderId = std::get<0>(turns);
            }
        }
    }
    return highestCardHolderId;
}

void clientLobbyManager::managementGAMEFIRSTTURNSERVERReceived(){
    constants::initializeCards(flushedCards);
    for(auto&g: gamePlayers){
        waitingForTurn.emplace_back(std::get<0>(g));
    }
    messageTypeExpected.clear();
    messageTypeExpected.emplace_back(lobbyMessageType::CHATMESSAGEID);
    messageTypeExpected.emplace_back(lobbyMessageType::GAMETURNSERVER);

    //Printing Starts
    gamePF::setTurnSequence(gamePlayers, turnSequence);

    gamePF::setRoundTurns(roundTurns, gamePlayers);

    gamePF::setWaitingForTurn(waitingForTurn, gamePlayers);
    gamePF::setCards(myCards);

    sati::getInstance()->setBase(this, appState::GAME);
    //set input statement and print all this

    if(myCards.find(deckSuit::SPADE)->second.empty()){
        //badranga
        assignToTurnAbleCards();
    }else{
        if(myCards.find(deckSuit::SPADE)->second.find(0) != myCards.find(deckSuit::SPADE)->second.end()){
            turnAbleCards.clear();
            turnAbleCards.emplace_back(Card(deckSuit::SPADE, 0));
        }else{
            assignToTurnAbleCards(deckSuit::SPADE);
        }
    }
    gamePF::setInputStatementHome3Accumulate();
    turnPlayerIdExpected = id;

    inputTypeExpected = inputType::GAMEINT;
    sati::getInstance()->setInputType(inputType::GAMEINT);
    if(waitingForTurn.empty()){
        firstRound = false;
    }
    else{
        firstRound = true;
    }
    gameStarted = true;
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