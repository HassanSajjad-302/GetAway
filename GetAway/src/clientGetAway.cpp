#include <algorithm>
#include <utility>
#include "clientGetAway.hpp"
#include "messageTypeEnums.hpp"
#include "sati.hpp"
#include "constants.hpp"
#include "resourceStrings.hpp"
#include "clientLobby.hpp"
clientGetAway::clientGetAway(clientLobby &lobbyManager_, const std::string& playerName_,
                             const std::map<int, std::string>& players_, std::istream& in, int myId_, bool clientOnly_):
        lobby{lobbyManager_}, playerName{playerName_}, players{players_}, myId{myId_}, clientOnly(clientOnly_)
{
    constants::initializeCards(myCards);
    constants::initializeCards(flushedCards);

    //STEP 1;
    int handSize = 0;
    //STEP 2;
    in.read(reinterpret_cast<char*>(&handSize), sizeof(handSize));
    assert((handSize <= ((constants::DECKSIZE / players.size()) + 1) &&
            handSize >= ((constants::DECKSIZE / players.size()) - 1)) &&
           "Unexpected Number Of Cards Received");
    for(int i=0; i < handSize; ++i){
        Card card;
        in.read(reinterpret_cast<char *>(&card), sizeof(card));
        myCards.find(card.suit)->second.emplace(card.cardNumber);
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

    for(auto&g: players){
        waitingForTurn.emplace_back(std::get<0>(g));
    }

    //Printing Starts
    PF::setTurnSequence(players, turnSequence);

    PF::setRoundTurns(roundTurns, players);

    PF::setWaitingForTurn(waitingForTurn, players);
    PF::setCards(myCards);

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
    PF::setInputStatementHome3Accumulate(clientOnly);
    turnPlayerIdExpected = myId;

    inputTypeExpected = inputType::OPTIONSELECTIONINPUTGAME;
    sati::getInstance()->setInputType(inputType::OPTIONSELECTIONINPUTGAME);
    if(waitingForTurn.empty()){
        firstRound = false;
    }
    else{
        firstRound = true;
    }
}

void clientGetAway::packetReceivedFromNetwork(std::istream &in, int receivedPacketSize) {

    mtgg messageTypeReceived;
    in.read(reinterpret_cast<char*>(&messageTypeReceived), sizeof(messageType));
    switch(messageTypeReceived){
        //STEP 1;
        case mtgg::GAMETURNSERVER:{
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
            resourceStrings::print("Unexpected Packet Type Received in class clientGetAway "
                                   "message type not in enum mtgg\r\n");
            break;
        }
    }
}

void clientGetAway::sendGAMETURNCLIENT(Card card){
    constants::Log("Seding Card Message To Server. Suit {} {}",
                   deckSuitValue::displaySuitType[(int)card.suit], deckSuitValue::displayCards[card.cardNumber]);
    std::ostream& out = lobby.clientLobbySession.out;
    //STEP 1;
    out.write(reinterpret_cast<const char*>(&constants::mtcGame), sizeof(constants::mtcGame));
    mtgg t = mtgg::GAMETURNCLIENT;
    out.write(reinterpret_cast<char*>(&t), sizeof(t));
    //STEP 2;
    out.write(reinterpret_cast<char *>(&card), sizeof(card));
    lobby.clientLobbySession.sendMessage();
}

void clientGetAway::input(std::string inputString, inputType inputReceivedType) {
    if(inputReceivedType == inputTypeExpected){
        switch (inputReceivedType) {
            case inputType::OPTIONSELECTIONINPUTGAME:
            {
                int input;
                bool success;
                if(firstRound){
                    if(std::find(waitingForTurn.begin(), waitingForTurn.end(), myId) == waitingForTurn.end()){
                        //don't perform turn
                        success = constants::inputHelper(inputString, 1, 3, inputType::OPTIONSELECTIONINPUTGAME, inputType::OPTIONSELECTIONINPUTGAME,
                                                         input);
                    }else{
                        success = constants::inputHelper(inputString, 1, 4, inputType::OPTIONSELECTIONINPUTGAME, inputType::OPTIONSELECTIONINPUTGAME,
                                                         input);
                    }
                }else{
                    if(turnPlayerIdExpected != myId){
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
                        lobby.clientChatPtr->setBaseAndInputTypeForMESSAGESTRING();
                    }else if(input == 2){
                        lobby.exitApplication(true);
                    }else if(input == 3){
                        lobby.exitApplication(false);
                    }else{
                        //perform turn
                        PF::setInputStatementHome3R3Accumulate(turnAbleCards);
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
                        Turn(myId, turnAbleCards[input - 1], whoTurned::CLIENT);
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

void clientGetAway::setBaseAndInputTypeFromclientLobby(){
    setInputTypeGameInt();
    setBaseAndInputType(this, inputType::OPTIONSELECTIONINPUTGAME);
}

void clientGetAway::setInputTypeGameInt(){
    if(firstRound){
        if(std::find(waitingForTurn.begin(), waitingForTurn.end(), myId) == waitingForTurn.end()){
            PF::setInputStatementHome2Accumulate(clientOnly);
        }else{
            PF::setInputStatementHome3Accumulate(clientOnly);
        }
    }else{
        if(turnPlayerIdExpected != myId){
            PF::setInputStatementHome2Accumulate(clientOnly);
        }else{
            PF::setInputStatementHome3Accumulate(clientOnly);
        }
    }
}
void clientGetAway::firstRoundTurnHelper(int playerId, Card card, whoTurned who){
    constants::Log("firstRoundTurnHelper Called");
    waitingForTurn.erase(std::find(waitingForTurn.begin(), waitingForTurn.end(), playerId));
    if(who == whoTurned::CLIENT){
        myCards.find(card.suit)->second.erase(card.cardNumber);
        PF::setCards(myCards);
        setInputTypeGameInt();
        setInputType(inputType::OPTIONSELECTIONINPUTGAME);
    }
    flushedCards.find(card.suit)->second.emplace(card.cardNumber);
    roundTurns.emplace_back(playerId, card);
    PF::setWaitingForTurn(waitingForTurn, players);
    PF::setRoundTurnsAccumulate(roundTurns, players);
    numberOfCards.find(playerId)->second -= 1;

    if (roundTurns.size() == turnSequence.size()) {
        assert(waitingForTurn.empty() && "A New Round Is Starting When Old Round Turns Are Not Received Yet.");
        for (auto t: roundTurns) {
            if (std::get<1>(t).suit == deckSuit::SPADE && std::get<1>(t).cardNumber == 0) {
                //This is ace of spade
                turnPlayerIdExpected = std::get<0>(t);
                waitingForTurn.emplace_back(std::get<0>(t));
                PF::setWaitingForTurn(waitingForTurn, players);
                firstRound = false;
                //Here I am auto turning. So, here I have to check whether auto turn is possible and if yes then
                //auto turn but do not send it to the server and just clearAndPrint it to the screen.
                if(myId == turnPlayerIdExpected){
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
void clientGetAway::Turn(int playerId, Card card, whoTurned who) {
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
        if(gameFinished){
            lobby.gameFinished();
        }else{
            waitingForTurn.emplace_back(turnPlayerIdExpected);
            PF::setWaitingForTurn(waitingForTurn, players);
            setInputTypeGameInt();
            setInputType(inputType::OPTIONSELECTIONINPUTGAME);
        }
    }else{
        constants::Log("No Message Was Expected From This User");
    }
}

void clientGetAway::helperFirstTurnAndMiddleTurn(int playerId, Card card, bool firstTurn, whoTurned who) {
    constants::Log("FirstTurnAndMiddleTurn Called. firstTurn value is {}", std::to_string(firstTurn));
    if(firstTurn){
        suitOfTheRound = card.suit;
        constants::Log("Suit Of The Round is {}", deckSuitValue::displaySuitType[(int)suitOfTheRound]);
    }
    roundTurns.emplace_back(playerId, card);
    PF::setRoundTurns(roundTurns, players);

    if(playerId == myId){
        myCards.find(card.suit)->second.erase(card.cardNumber);
        PF::setCards(myCards);
    }


    turnPlayerIdExpected = nextInTurnSequence(playerId);
    constants::Log("turnPlayerIdExpected is {}", turnPlayerIdExpected);
    if(turnPlayerIdExpected == myId){
        if(myCards.find(card.suit)->second.empty()){
            assignToTurnAbleCards();
        }else{
            assignToTurnAbleCards(suitOfTheRound);
        }
    }
}

void clientGetAway::helperLastTurnAndThullaTurn(int playerId, Card card, bool thullaTurn, whoTurned who) {
    constants::Log("helperLastTurnAndThullaTurn Called. thullaTurn is {}", thullaTurn);
    if(thullaTurn){
        turnPlayerIdExpected = roundKing();
        constants::Log("Thulla Receival Id is {}", turnPlayerIdExpected);
        //There are two possibilities and we are interested in both. roundking may be our id or the
        //playerId may be our Id.
        assert(turnPlayerIdExpected != playerId && "Why we are performing thulla turn if we have already performed our turn in the round");
        roundTurns.emplace_back(playerId, card);
        PF::setRoundTurns(roundTurns, players);

        if(playerId == myId){
            myCards.find(card.suit)->second.erase(card.cardNumber);
            PF::setCards(myCards);
        }else if(turnPlayerIdExpected == myId){
            constants::Log("I received a thulla");
            for(auto thullaCards: roundTurns){
                myCards.find(std::get<1>(thullaCards).suit)->second.emplace(std::get<1>(thullaCards).cardNumber);
            }
            PF::setCards(myCards);
        }

        for(auto& rt: roundTurns){
            numberOfCards.find(std::get<0>(rt))->second -= 1;
        }
        numberOfCards.find(turnPlayerIdExpected)->second += roundTurns.size();
    }else{
        roundTurns.emplace_back(playerId, card);
        PF::setRoundTurns(roundTurns, players);

        turnPlayerIdExpected = roundKing();
        if(playerId == myId){
            myCards.find(card.suit)->second.erase(card.cardNumber);
            PF::setCards(myCards);
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
        gameFinished = true;
        return;
    }else if(turnSequence.size() == 1){
        int loosingId = *turnSequence.begin();
        //player lost. move back to lobby
        resourceStrings::print("Player " + players.find(loosingId)->second + " Lost\r\n");
        gameFinished = true;
        return;
    }else{
        //game continues
        PF::setTurnSequence(players, turnSequence);

        if(turnPlayerIdExpected == myId){
            assignToTurnAbleCards();
        }
    }
}

int clientGetAway::nextInTurnSequence(int currentSessionId){
    std::vector<int>::iterator it;
    for(it = turnSequence.begin(); it != turnSequence.end(); ++it){
        if(*it == currentSessionId){
            break;
        }
    }
    assert(it != turnSequence.end() && "Id of current serverSession is not present in turnSequence");
    auto next = ++it;
    if(next != turnSequence.end()){
        return *next;
    }
    return *turnSequence.begin();
}

int clientGetAway::roundKing() {
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

void clientGetAway::setInputType(inputType inputType) {
    inputTypeExpected = inputType;
    sati::getInstance()->setInputType(inputType);
}

void clientGetAway::setBaseAndInputType(terminalInputBase *base_, inputType type) {
    inputTypeExpected = type;
    sati::getInstance()->setBaseAndInputType(base_, type);
}

void clientGetAway::assignToTurnAbleCards(){
    turnAbleCards.clear();
    for(auto& cardPair: myCards){
        for(auto c: cardPair.second){
            turnAbleCards.emplace_back(static_cast<deckSuit>(cardPair.first), c);
        }
    }
}

void clientGetAway::assignToTurnAbleCards(deckSuit suit) {
    turnAbleCards.clear();
    for(auto c: myCards.find(suit)->second){
        turnAbleCards.emplace_back(suit, c);
    }
}