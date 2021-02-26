#include <algorithm>
#include <utility>
#include "clientBluff.hpp"
#include "messageTypeEnums.hpp"
#include "sati.hpp"
#include "constants.h"
#include "resourceStrings.hpp"
#include "clientLobby.hpp"
Bluff::clientBluff::clientBluff(clientLobby &lobbyManager_, const std::string& playerName_,
                                const std::map<int, std::string>& players_, std::istream& in, int myId_, bool clientOnly_):
        lobbyManager{lobbyManager_}, playerName{playerName_}, players{players_}, myId{myId_}, clientOnly(clientOnly_)
{
    constants::initializeCards(myCards);

    //STEP 1;
    int handSize = 0;
    //STEP 2;
    in.read(reinterpret_cast<char*>(&handSize), sizeof(handSize));
    assert((handSize <= ((constants::DECKSIZE / players.size()) + 1) &&
            handSize >= ((constants::DECKSIZE / players.size()) - 1)) &&
           "Unexpected Number Of Cards Received");
    for(int i=0; i < handSize; ++i){
        Card card;
        //STEP 3;
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

    //STAGE 3;
    for(int i=0; i < players.size(); ++i){
        int gpId;
        int gpSize;
        //STEP 9;
        in.read(reinterpret_cast<char*>(&gpId), sizeof(gpId));
        //STEP 10;
        in.read(reinterpret_cast<char*>(&gpSize), sizeof(gpSize));
        numberOfCards.emplace(gpId, gpSize);
    }

    //Here we need to set some variable which depicts that first turn of the round is expected.
    firstTurnOfRoundExpected = true;
    passTurnCount = 0;
    cardsOnTableCount = 0;
    whoWillTurnNext_TurnSequenceIndex = 0;
    aPlayerIsMarkedForRemoval = false;

    //Printing Starts
    PF::setTurnSequence(players, turnSequence);
    PF::setRoundTurns(roundTurns, players);
    PF::setWaitingForTurn(turnSequence[0], players);
    PF::setCards(myCards);


    if(turnSequence[whoWillTurnNext_TurnSequenceIndex] == myId){
        PF::promptFirstTurnAccumulate(clientOnly);
        setBaseAndInputType(this,inputType::GB_FIRSTTURNOFTHEROUND);
    }else{
        PF::promptSimpleNonTurnInput(clientOnly);
        setBaseAndInputType(this,inputType::GB_SIMPLENONTURNINPUT);
    }
}

void Bluff::clientBluff::packetReceivedFromNetwork(std::istream &in, int receivedPacketSize) {
    if (firstTurnOfRoundExpected) {
        //STEP 1;
        in.read(reinterpret_cast<char *>(&suitOfTheRound), sizeof(suitOfTheRound));
        in.read(reinterpret_cast<char *>(&numberOfCardsTurned), sizeof(numberOfCardsTurned));

        performFIRSTTurn();
    }else{
        mtgb messageTypeReceived;
        in.read(reinterpret_cast<char *>(&messageTypeReceived), sizeof(messageType));
        switch (messageTypeReceived) {
            case mtgb::SERVER_CLIENTTURNNORMAL: {
                in.read(reinterpret_cast<char *>(&numberOfCardsTurned), sizeof(numberOfCardsTurned));
                performNORMALTurn();
                break;
            }
            case mtgb::SERVER_CLIENTTURNCHECK: {
                auto cardsReceived = std::make_unique<Card[]>(numberOfCardsTurned);
                in.read(reinterpret_cast<char *>(&cardsReceived), sizeof(cardsReceived));

                break;
            }
            case mtgb::SERVER_CLIENTTURNPASS:{
                performPASSTurn();
            }
            default: {
                resourceStrings::print("Unexpected Packet Type Received in class clientLobbyManager "
                                       "message type not in enum mtgg\r\n");
                break;
            }
        }
    }
}

void Bluff::clientBluff::performFIRSTTurn(){
    cardsOnTableCount += numberOfCardsTurned;
    firstTurnOfRoundExpected = false;
    lastPlayerWhoTurnedId = turnSequence[whoWillTurnNext_TurnSequenceIndex];
    passTurnCount = 0;

    numberOfCards.find(turnSequence[whoWillTurnNext_TurnSequenceIndex])->second -= numberOfCardsTurned;
    if(numberOfCards.find(turnSequence[whoWillTurnNext_TurnSequenceIndex])->second == 0){
        aPlayerIsMarkedForRemoval = true;
        markedPlayerForRemovalId = turnSequence[whoWillTurnNext_TurnSequenceIndex];
    }
    roundTurns.emplace_back(std::make_tuple(turnSequence[whoWillTurnNext_TurnSequenceIndex],
                                            bluffTurn(Bluff::turnType::FIRST, suitOfTheRound, numberOfCardsTurned)));
    incrementWhoWillTurnNext_TurnSequenceIndex();
    PF::setRoundTurns(roundTurns, players);
    if(turnSequence[whoWillTurnNext_TurnSequenceIndex] == myId){
        //It should be a normal turn as it is a second turn.
        PF::promptNormalTurnAccumulate(clientOnly);
        setInputType(inputType::GB_NORMALTURNOFTHEROUND);
    }else{
        PF::promptSimpleNonTurnInput(clientOnly);
        setInputType(inputType::GB_SIMPLENONTURNINPUT);
    }
}

void Bluff::clientBluff::performNORMALTurn(){

    cardsOnTableCount += numberOfCardsTurned;
    passTurnCount = 0;
    lastPlayerWhoTurnedId = turnSequence[whoWillTurnNext_TurnSequenceIndex];

    numberOfCards.find(turnSequence[whoWillTurnNext_TurnSequenceIndex])->second -= numberOfCardsTurned;
    if(aPlayerIsMarkedForRemoval){
        aPlayerIsMarkedForRemoval = false;
        int index = 0;
        for(int i=0;i<turnSequence.size(); ++i){
            if(turnSequence[i] == markedPlayerForRemovalId){
                index = i;
                break;
            }
        }
        turnSequence.erase(turnSequence.begin(), turnSequence.begin() + index);
    }
    if(numberOfCards.find(turnSequence[whoWillTurnNext_TurnSequenceIndex])->second == 0){

        aPlayerIsMarkedForRemoval = true;
        markedPlayerForRemovalId = turnSequence[whoWillTurnNext_TurnSequenceIndex];
    }

    roundTurns.emplace_back(std::make_tuple(turnSequence[whoWillTurnNext_TurnSequenceIndex],
                                                            bluffTurn(Bluff::turnType::NORMAL, numberOfCardsTurned)));
    incrementWhoWillTurnNext_TurnSequenceIndex();

    PF::setRoundTurns(roundTurns, players);
    if(turnSequence[whoWillTurnNext_TurnSequenceIndex] == myId){
        //It should be a normal turn as it is a second turn.
        PF::promptNormalTurnAccumulate(clientOnly);
        setInputType(inputType::GB_NORMALTURNOFTHEROUND);
    }else{
        PF::promptSimpleNonTurnInput(clientOnly);
        setInputType(inputType::GB_SIMPLENONTURNINPUT);
    }
}

void Bluff::clientBluff::performCHECKTurn(Card* cardsReceived){

    firstTurnOfRoundExpected = true;
    passTurnCount = 0;

    bool bluff = false;
    for(int i=0; i<numberOfCardsTurned; ++i){
        if(cardsReceived[i].suit != suitOfTheRound){
            bluff = true;
            break;
        }
    }

    if(bluff){
        if(aPlayerIsMarkedForRemoval){
            aPlayerIsMarkedForRemoval = false;
            assert(aPlayerIsMarkedForRemoval == lastPlayerWhoTurnedId
                   && "If a player id is marked for removal, then it's id should be equal to the lastplayerwhoturned "
                      "id. Otherwise it suggests that aPlayerIsMarkedForRemoval was not falsed after someone else "
                      "played cards after setting it to true which breaks the invariant.");
        }
        numberOfCards.find(lastPlayerWhoTurnedId)->second += cardsOnTableCount;
        for(int i=0;i<turnSequence.size(); ++i){
            if(lastPlayerWhoTurnedId == turnSequence[i]){
                whoWillTurnNext_TurnSequenceIndex = i;
                break;
            }
        }
    }else{
        numberOfCards.find(turnSequence[whoWillTurnNext_TurnSequenceIndex])->second += cardsOnTableCount;
    }
    if(turnSequence[whoWillTurnNext_TurnSequenceIndex] == myId){
        //It is our turn and it is first turn.
        PF::promptFirstTurnAccumulate(clientOnly);
        setInputType(inputType::GB_FIRSTTURNOFTHEROUND);
    }
}

void Bluff::clientBluff::performPASSTurn(){
    ++passTurnCount;
    if(aPlayerIsMarkedForRemoval){
        if(passTurnCount == turnSequence.size() - 1){
            aPlayerIsMarkedForRemoval = false;
            firstTurnOfRoundExpected = true;
            cardsOnTableCount = 0;
            roundTurns.emplace_back(std::make_tuple(turnSequence[whoWillTurnNext_TurnSequenceIndex],
                                                    bluffTurn(Bluff::turnType::PASS)));
            incrementWhoWillTurnNext_TurnSequenceIndex();
            turnSequence.erase(turnSequence.begin(),
                               turnSequence.begin() + whoWillTurnNext_TurnSequenceIndex);
            incrementWhoWillTurnNext_TurnSequenceIndex();
            PF::setRoundTurns(roundTurns, players);
            if(turnSequence[whoWillTurnNext_TurnSequenceIndex] == myId){
                //It should be a normal turn as it is a second turn.
                PF::promptNormalTurnAccumulate(clientOnly);
                setInputType(inputType::GB_NORMALTURNOFTHEROUND);
            }else{
                PF::promptSimpleNonTurnInput(clientOnly);
                setInputType(inputType::GB_SIMPLENONTURNINPUT);
            }
        }
    }else if(passTurnCount == turnSequence.size()){
        firstTurnOfRoundExpected = true;
        cardsOnTableCount = 0;
        roundTurns.emplace_back(std::make_tuple(turnSequence[whoWillTurnNext_TurnSequenceIndex],
                                                bluffTurn(Bluff::turnType::PASS)));
        incrementWhoWillTurnNext_TurnSequenceIndex();
        PF::setRoundTurns(roundTurns, players);
        if(turnSequence[whoWillTurnNext_TurnSequenceIndex] == myId){
            //It should be a normal turn as it is a second turn.
            PF::promptNormalTurnAccumulate(clientOnly);
            setInputType(inputType::GB_NORMALTURNOFTHEROUND);
        }else{
            PF::promptSimpleNonTurnInput(clientOnly);
            setInputType(inputType::GB_SIMPLENONTURNINPUT);
        }
    }
}

void Bluff::clientBluff::input(std::string inputString, inputType inputReceivedType) {
    if (inputReceivedType == inputTypeExpected) {
        if(inputReceivedType == inputType::GB_SIMPLENONTURNINPUT ||
        inputReceivedType == inputType::GB_FIRSTTURNOFTHEROUND ||
        inputReceivedType == inputType::GB_NORMALTURNOFTHEROUND){
            int input;
            bool success;
            if(inputReceivedType == inputType::GB_SIMPLENONTURNINPUT){
                success = constants::inputHelper(inputString, 1, 3, inputType::GB_SIMPLENONTURNINPUT,
                                                 inputType::GB_SIMPLENONTURNINPUT,
                                                 input);
            }
            else if(inputReceivedType == inputType::GB_FIRSTTURNOFTHEROUND){
                success = constants::inputHelper(inputString, 1, 4, inputType::GB_FIRSTTURNOFTHEROUND,
                                                 inputType::GB_FIRSTTURNOFTHEROUND,
                                                 input);
            }else{
                success = constants::inputHelper(inputString, 1, 6, inputType::GB_NORMALTURNOFTHEROUND,
                                                 inputType::GB_NORMALTURNOFTHEROUND,
                                                 input);
            }
            if(success){
                if (input == 1) {
                    //SEND MESSAGE
                    lobbyManager.clientChatPtr->setBaseAndInputTypeForMESSAGESTRING();
                } else if (input == 2) {
                    //LEAVE || CLOSE SERVER
                    lobbyManager.exitApplication(true);
                } else if (input == 3) {
                    //EXIT APPLICATION
                    lobbyManager.exitApplication(false);
                } else if (input == 4) {
                    //PLAY CARDS
                    PF::promptFirstTurnOrNormalTurnSelectCards(myCards);
                    setInputType(inputType::GB_PERFORMNORMALTURNSELECTCARDS);
                } else if (input == 5) {
                    //CHECK
                    std::ostream& out = lobbyManager.clientLobbySession.out;
                    mtgb turnType = mtgb::CLIENT_TURNCHECK;
                    out.write(reinterpret_cast<char*>(&turnType), sizeof(turnType));
                    lobbyManager.clientLobbySession.sendMessage();

                    PF::promptAndInputWaitingForCardsAccumulate(clientOnly);
                    setInputType(inputType::GB_SIMPLENONTURNINPUT);
                } else if (input == 6) {
                    //PASS
                    std::ostream& out = lobbyManager.clientLobbySession.out;
                    mtgb turnType = mtgb::CLIENT_TURNPASS;
                    out.write(reinterpret_cast<char*>(&turnType), sizeof(turnType));
                    lobbyManager.clientLobbySession.sendMessage();
                    roundTurns.emplace_back(std::make_tuple(myId, bluffTurn(Bluff::turnType::PASS)));
                    performPASSTurn();
                }
            }
        }else if (inputReceivedType == inputType::GB_PERFORMFIRSTTURNSELECTCARDS) {
            if(inputHelperInCardsSelection(inputString, inputType::GB_FIRSTTURNOFTHEROUND)){
                PF::promptFirsTurnSelectDeckSuitAccumulate();
                setInputType(inputType::GB_PERFORMFIRSTTURNSELECTDECKSUITTYPE);
            }
        }else if(inputReceivedType == inputType::GB_PERFORMFIRSTTURNSELECTDECKSUITTYPE){
            int input;
            bool  success = constants::inputHelper(inputString, 1, 4,
                                                   inputType::GB_PERFORMFIRSTTURNSELECTDECKSUITTYPE,
                                                   inputType::GB_PERFORMFIRSTTURNSELECTDECKSUITTYPE,
                                                   input);
            if(success){
                --input;
                auto suit = (deckSuit)input;

                //Here we have suit and cardNumbers. We have to turn these.
                std::ostream& out = lobbyManager.clientLobbySession.out;
                out.write(reinterpret_cast<char*>(&suit), sizeof(suit));
                int sz = selectedCardsIndicesFor_myCardsVectorLatest.size();
                out.write(reinterpret_cast<char*>(&sz), sizeof(sz));
                for(int i=0;i<selectedCardsIndicesFor_myCardsVectorLatest.size(); ++i){
                    out.write(reinterpret_cast<char *>(&myCardsVectorLatest[i]), sizeof(myCardsVectorLatest[i]));
                }
                lobbyManager.clientLobbySession.sendMessage();

                suitOfTheRound = suit;
                numberOfCardsTurned = sz;
                performFIRSTTurn();

            }
        }else if(inputReceivedType == inputType::GB_PERFORMNORMALTURNSELECTCARDS){
            if(inputHelperInCardsSelection(inputString, inputType::GB_NORMALTURNOFTHEROUND)){
               //TODO
               //Here we have card numbers and we have to perform turn.
                std::ostream& out = lobbyManager.clientLobbySession.out;
                auto turnType = mtgb::CLIENT_TURNNORMAL;
                out.write(reinterpret_cast<char*>(&turnType), sizeof(turnType));
                int sz = selectedCardsIndicesFor_myCardsVectorLatest.size();
                out.write(reinterpret_cast<char*>(&sz), sizeof(sz));
                for(int i=0;i<selectedCardsIndicesFor_myCardsVectorLatest.size(); ++i){
                    out.write(reinterpret_cast<char *>(&myCardsVectorLatest[i]), sizeof(myCardsVectorLatest[i]));
                }
                lobbyManager.clientLobbySession.sendMessage();

                numberOfCardsTurned = sz;
                performNORMALTurn();
            }
        }
    } else {
        resourceStrings::print("InputReceived Type not same as Input Received Type Expected\r\n");
    }
}

bool Bluff::clientBluff::inputHelperInCardsSelection(const std::string &inputString, inputType inputtype){
    std::string buf;                 // Have a buffer string
    std::stringstream ss(inputString);       // Insert the string into a stream

    myCardsVectorLatest.clear();
    selectedCardsIndicesFor_myCardsVectorLatest.clear();
    for(auto& cardPair: myCards){
        for(auto c: cardPair.second){
            myCardsVectorLatest.emplace_back(static_cast<deckSuit>(cardPair.first), c);
        }
    }
    while (ss >> buf){
        int input;
        bool  success = constants::inputHelper(buf, 1, myCardsVectorLatest.size(),
                                               inputType::GB_FIRSTTURNOFTHEROUND,
                                               inputType::GB_FIRSTTURNOFTHEROUND,
                                               input);
        if(!success){
            resourceStrings::print(buf + " is an invalid input");
            return false;
        }
        --input;
        selectedCardsIndicesFor_myCardsVectorLatest.emplace_back(input);
    }
    return true;
}

void Bluff::clientBluff::setBaseAndInputTypeFromclientChatMessage(){
    if(turnSequence[whoWillTurnNext_TurnSequenceIndex] == myId){
        if(firstTurnOfRoundExpected){
            PF::promptFirstTurnAccumulate(clientOnly);
            setBaseAndInputType(this, inputType::GB_FIRSTTURNOFTHEROUND);
        }else{
            PF::promptNormalTurnAccumulate(clientOnly);
            setBaseAndInputType(this, inputType::GB_NORMALTURNOFTHEROUND);
        }
    }else{
        PF::promptSimpleNonTurnInput(clientOnly);
        setBaseAndInputType(this, inputType::GB_SIMPLENONTURNINPUT);
    }
}

int Bluff::clientBluff::nextInTurnSequence(int currentSessionId){
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

void Bluff::clientBluff::setInputType(inputType inputType) {
    inputTypeExpected = inputType;
    sati::getInstance()->setInputType(inputType);
}

void Bluff::clientBluff::setBaseAndInputType(terminalInputBase *base_, inputType type) {
    inputTypeExpected = type;
    sati::getInstance()->setBaseAndInputType(base_, type);
}

void Bluff::clientBluff::incrementWhoWillTurnNext_TurnSequenceIndex(){
    ++whoWillTurnNext_TurnSequenceIndex;
    if(whoWillTurnNext_TurnSequenceIndex == turnSequence.size()){
        whoWillTurnNext_TurnSequenceIndex = 0;
    }
}

Bluff::bluffTurn::bluffTurn(turnType firstTurn_, deckSuit firstTurnSuit_, int firstTurnCardCount_):
        turn(firstTurn_), firstTurnSuit(firstTurnSuit_), cardCount(firstTurnCardCount_){

}

Bluff::bluffTurn::bluffTurn(turnType passTurn_): turn(passTurn_) {

}

Bluff::bluffTurn::bluffTurn(turnType normalTurn_, int normalTurnCardCount_): turn(normalTurn_), cardCount(normalTurnCardCount_) {

}

Bluff::bluffTurn::bluffTurn(turnType checkTurn_, std::vector<Card> checkTurn_LastTurnCards_): turn(checkTurn_),
                                                                                              checkTurn_LastTurnCards(std::move(checkTurn_LastTurnCards_)) {

}
