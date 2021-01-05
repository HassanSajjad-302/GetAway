#include <algorithm>
#include <utility>
#include <clientHome.hpp>
#include "clientLobbyManager.hpp"
#include "messageTypeEnums.hpp"
#include "sati.hpp"

#include "constants.h"
#include "gamePF.hpp"
#include "messagePF.hpp"
#include "resourceStrings.hpp"

clientLobbyManager::clientLobbyManager(clientRoomManager &roomManager_, const std::string& playerName_, const std::map<int, std::string>& players_):
        roomManager{roomManager_}, playerName{playerName_}, players{players_}
{
    constants::initializeCards(myCards);
}

clientLobbyManager::~clientLobbyManager() = default;

void clientLobbyManager::join(std::shared_ptr<session<clientLobbyManager>> clientLobbySession_) {
    messageTypeExpected.clear();
    messageTypeExpected.emplace_back(mtg::GAMEFIRSTTURNSERVER);
}

void clientLobbyManager::packetReceivedFromNetwork(std::istream &in, int receivedPacketSize) {

    mtg messageTypeReceived;
    in.read(reinterpret_cast<char*>(&messageTypeReceived), sizeof(messageType));
    if(std::find(messageTypeExpected.begin(), messageTypeExpected.end(), messageTypeReceived) == messageTypeExpected.end()){
        resourceStrings::print("Unexpected Packet Type Received in class clientLobbyManager\r\n");
    }else{
        switch(messageTypeReceived){
                //STEP 1;
            case mtg::GAMEFIRSTTURNSERVER:{
                int handSize = 0;
                //STEP 2;
                in.read(reinterpret_cast<char*>(&handSize), sizeof(handSize));
                assert((handSize <= ((constants::DECKSIZE / players.size()) + 1) &&
                        handSize >= ((constants::DECKSIZE / players.size()) - 1)) &&
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
                for(int i=0; i < players.size(); ++i){
                    int sequenceId;
                    //STEP 5; //turn sequence
                    in.read(reinterpret_cast<char*>(&sequenceId), sizeof(sequenceId));
                    assert(players.find(sequenceId) != players.end() && "turn SequenceId not present in gamePlayerid");
                    turnSequence.emplace_back(sequenceId);
                }

                for(int i=0; i < players.size(); ++i){
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
            case mtg::GAMETURNSERVER:{
                int senderId;
                deckSuit suit;
                int cardNumber;

                //STEP 2;
                in.read(reinterpret_cast<char*>(&senderId), sizeof(senderId));
                //STEP 3;
                in.read(reinterpret_cast<char *>(&suit), sizeof(suit));
                //STEP 4;
                in.read(reinterpret_cast<char*>(&cardNumber), sizeof(cardNumber));
                constants::Log("GameTurnServerReceived From Id {}. Card Is {} {}",
                             senderId, deckSuitValue::displaySuitType[(int) suit], deckSuitValue::displayCards[cardNumber]);
                Turn(senderId, Card(suit, cardNumber), whoTurned::RECEIVED);
                break;
            }
            default:{
                resourceStrings::print("Unexpected Packet Type Received in class serverRoomManager\r\n");
                break;
            }
        }
    }
}

void clientLobbyManager::sendGAMETURNCLIENT(Card card){
    std::ostream& out = roomManager.clientRoomSession->out;
    //STEP 1;
    mtg t = mtg::GAMETURNCLIENT;
    constants::Log("Seding Card Message To Server. Suit {} {}",
                 deckSuitValue::displaySuitType[(int)card.suit], deckSuitValue::displayCards[card.cardNumber]);
    out.write(reinterpret_cast<char*>(&t), sizeof(t));
    //TODO
    //Check if I can send and receive card in one go.
    //STEP 2;
    out.write(reinterpret_cast<char *>(&card.suit), sizeof(card.suit));
    //STEP 2;
    out.write(reinterpret_cast<char *>(&card.cardNumber), sizeof(card.cardNumber));

    roomManager.clientRoomSession->sendMessage();
}

//Before refactor, lines of this function = 123;
void clientLobbyManager::input(std::string inputString, inputType inputReceivedType) {
    if(inputReceivedType == inputTypeExpected){
        switch (inputReceivedType) {
            case inputType::OPTIONSELECTIONINPUTGAME:
            {
                int input;
                bool success;
                if(firstRound){
                    if(std::find(waitingForTurn.begin(), waitingForTurn.end(), id) == waitingForTurn.end()){
                        //don't perform turn
                        success = constants::inputHelper(inputString, 1, 3, inputType::OPTIONSELECTIONINPUTGAME, inputType::OPTIONSELECTIONINPUTGAME,
                                                         input);
                    }else{
                        success = constants::inputHelper(inputString, 1, 4, inputType::OPTIONSELECTIONINPUTGAME, inputType::OPTIONSELECTIONINPUTGAME,
                                                         input);
                    }
                }else{
                    if(turnPlayerIdExpected != id){
                        //don't perform turn
                        success = constants::inputHelper(inputString, 1, 3, inputType::OPTIONSELECTIONINPUTGAME, inputType::OPTIONSELECTIONINPUTGAME,
                                                         input);
                    }else{
                        success = constants::inputHelper(inputString, 1, 4, inputType::OPTIONSELECTIONINPUTGAME, inputType::OPTIONSELECTIONINPUTGAME,
                                                         input);
                    }
                }

                if(success){
                    if(input == 1){
                        messagePF::setInputStatementAccumulate();
                        setBaseAndInputType(roomManager.chatManager.get(), inputType::MESSAGESTRING);
                    }else if(input == 2){
                        roomManager.leaveGame();
                    }else if(input == 3){
                        roomManager.exitApplicationAmidGame();
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
                    setInputType(inputType::OPTIONSELECTIONINPUTGAME);
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

void clientLobbyManager::setBaseAndInputTypeFromclientChatMessage(){
    setInputTypeGameInt();
    setBaseAndInputType(this, inputType::OPTIONSELECTIONINPUTGAME);
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
}
void clientLobbyManager::firstRoundTurnHelper(int playerId, Card card, whoTurned who){
    constants::Log("firstRoundTurnHelper Called");
    waitingForTurn.erase(std::find(waitingForTurn.begin(), waitingForTurn.end(), playerId));
    if(who == whoTurned::CLIENT){
        myCards.find(card.suit)->second.erase(card.cardNumber);
        gamePF::setCards(myCards);
        setInputTypeGameInt();
        setInputType(inputType::OPTIONSELECTIONINPUTGAME);
    }
    flushedCards.find(card.suit)->second.emplace(card.cardNumber);
    roundTurns.emplace_back(playerId, card);
    gamePF::setWaitingForTurn(waitingForTurn, players);
    gamePF::setRoundTurnsAccumulate(roundTurns, players);
    numberOfCards.find(playerId)->second -= 1;

    if (roundTurns.size() == turnSequence.size()) {
        assert(waitingForTurn.empty() && "A New Round Is Starting When Old Round Turns Are Not Received Yet.");
        for (auto t: roundTurns) {
            if (std::get<1>(t).suit == deckSuit::SPADE && std::get<1>(t).cardNumber == 0) {
                //This is ace of spade
                turnPlayerIdExpected = std::get<0>(t);
                waitingForTurn.emplace_back(std::get<0>(t));
                gamePF::setWaitingForTurn(waitingForTurn, players);
                firstRound = false;
                //Here I am auto turning. So, here I have to check whether auto turn is possible and if yes then
                //auto turn but do not send it to the server and just clearAndPrint it to the screen.
                if(id == turnPlayerIdExpected){
                    assignToTurnAbleCards();
                }
                setInputTypeGameInt();
                setInputType(inputType::OPTIONSELECTIONINPUTGAME);
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
        waitingForTurn.clear();
        if(roundTurns.empty()){
            helperFirstTurnAndMiddleTurn(playerId, card, true, who);
        }else if(card.suit != suitOfTheRound){
            helperLastTurnAndThullaTurn(playerId, card, true, who);
        }else if(roundTurns.size() == turnSequence.size() - 1){
            helperLastTurnAndThullaTurn(playerId, card, false, who);
        }else{
            helperFirstTurnAndMiddleTurn(playerId, card, false, who);
        }
        waitingForTurn.emplace_back(turnPlayerIdExpected);
        gamePF::setWaitingForTurn(waitingForTurn, players);
        setInputTypeGameInt();
        setInputType(inputType::OPTIONSELECTIONINPUTGAME);
    }else{
        constants::Log("No Message Was Expected From This User");
    }
}

void clientLobbyManager::helperFirstTurnAndMiddleTurn(int playerId, Card card, bool firstTurn, whoTurned who) {
    constants::Log("FirstTurnAndMiddleTurn Called. firstTurn value is {}", std::to_string(firstTurn));
    if(firstTurn){
        suitOfTheRound = card.suit;
        constants::Log("Suit Of The Round is {}", deckSuitValue::displaySuitType[(int)suitOfTheRound]);
    }
    roundTurns.emplace_back(playerId, card);
    gamePF::setRoundTurns(roundTurns, players);

    if(playerId == id){
        myCards.find(card.suit)->second.erase(card.cardNumber);
        gamePF::setCards(myCards);
    }


    turnPlayerIdExpected = nextInTurnSequence(playerId);
    constants::Log("turnPlayerIdExpected is {}", turnPlayerIdExpected);
    if(turnPlayerIdExpected == id){
        if(myCards.find(card.suit)->second.empty()){
            assignToTurnAbleCards();
        }else{
            assignToTurnAbleCards(suitOfTheRound);
        }
    }
}

void clientLobbyManager::helperLastTurnAndThullaTurn(int playerId, Card card, bool thullaTurn, whoTurned who) {
    constants::Log("helperLastTurnAndThullaTurn Called. thullaTurn is {}", thullaTurn);
    if(thullaTurn){
        turnPlayerIdExpected = roundKing();
        constants::Log("Thulla Receival Id is {}", turnPlayerIdExpected);
        //There are two possibilities and we are interested in both. roundking may be our id or the
        //playerId may be our Id.
        assert(turnPlayerIdExpected != playerId && "Why we are performing thulla turn if we have already performed our turn in the round");
        roundTurns.emplace_back(playerId, card);
        gamePF::setRoundTurns(roundTurns, players);

        if(playerId == id){
            myCards.find(card.suit)->second.erase(card.cardNumber);
            gamePF::setCards(myCards);
        }else if(turnPlayerIdExpected == id){
            constants::Log("I received a thulla");
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
        gamePF::setRoundTurns(roundTurns, players);

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

    std::vector<int> removalIds;
    //Anyone whose cards are ended is removed from turnSequence.
    for(auto p: numberOfCards){
        int id1 = std::get<0>(p);
        int numOfCards = std::get<1>(p);
        if(numOfCards == 0){
            constants::Log("numberOfCards of id {} are 0", id1);
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
            removalIds.emplace_back(std::get<0>(p));
        }
    }
    for(auto removalId: removalIds){
        numberOfCards.erase(numberOfCards.find(removalId));
    }

    if(turnSequence.empty()){
        //game drawn. move back to lobby
        resourceStrings::print("Game Drawn\r\n");
        gameExitFinished();
        return;
    }else if(turnSequence.size() == 1){
        int loosingId = *turnSequence.begin();
        //player lost. move back to lobby
        resourceStrings::print("Player " + players.find(loosingId)->second + " Lost\r\n");
        gameExitFinished();
        return;
    }else{
        //game continues
        gamePF::setTurnSequence(players, turnSequence);

        if(turnPlayerIdExpected == id){
            assignToTurnAbleCards();
        }
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
    for(auto&g: players){
        waitingForTurn.emplace_back(std::get<0>(g));
    }
    messageTypeExpected.clear();
    messageTypeExpected.emplace_back(mtg::GAMETURNSERVER);

    //Printing Starts
    gamePF::setTurnSequence(players, turnSequence);

    gamePF::setRoundTurns(roundTurns, players);

    gamePF::setWaitingForTurn(waitingForTurn, players);
    gamePF::setCards(myCards);

    sati::getInstance()->setBase(this, appState::GAME);
    //set input statement and clearAndPrint all this

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

    inputTypeExpected = inputType::OPTIONSELECTIONINPUTGAME;
    sati::getInstance()->setInputType(inputType::OPTIONSELECTIONINPUTGAME);
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

void clientLobbyManager::setBaseAndInputType(terminalInputBase *base_, inputType type) {
    inputTypeExpected = type;
    sati::getInstance()->setBaseAndInputType(base_, type);
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

void clientLobbyManager::gameExitFinished(){
    constants::initializeCards(myCards);
    gameStarted = false;
    turnSequence.clear();
    turnAbleCards.clear();
    waitingForTurn.clear();
    numberOfCards.clear();
    messageTypeExpected.clear();
    messageTypeExpected.emplace_back(messageType::CHATMESSAGEID);
    messageTypeExpected.emplace_back(messageType::PLAYERJOINED);
    messageTypeExpected.emplace_back(messageType::PLAYERLEFT);
    messageTypeExpected.emplace_back(messageType::GAMEFIRSTTURNSERVER);
    sati::getInstance()->setBase(this, appState::LOBBY);
    lobbyPF::addOrRemovePlayer(players);
    lobbyPF::setInputStatementHomeAccumulate();
    setInputType(inputType::OPTIONSELECTIONINPUTLOBBY);
}