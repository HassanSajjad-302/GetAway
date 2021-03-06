#include <algorithm>
#include <utility>
#include <home.hpp>
#include "clientBluff.hpp"
#include "messageTypeEnums.hpp"
#include "sati.hpp"
#include "constants.hpp"
#include "resourceStrings.hpp"
#include "clientLobby.hpp"

void readMoreFailInClientSession();

Bluff::clientBluff::clientBluff(clientLobby &lobby_, const std::string& playerName_,
                                const std::map<int, std::string>& players_, std::istream& in, int myId_, bool clientOnly_):
        lobby{lobby_}, playerName{playerName_}, players{players_}, myId{myId_}, clientOnly(clientOnly_)
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
        PF::promptSimpleNonTurnInputAccumulate(clientOnly);
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
                std::vector<Card> cardsReceived;
                cardsReceived.resize(numberOfCardsTurned);
                in.read(reinterpret_cast<char *>(&cardsReceived[0]), sizeof(Card) * numberOfCardsTurned);
                performCHECKTurn(cardsReceived, false);
                break;
            }
            case mtgb::SERVER_BLUFFDETECTEDORWRONGCHECK:{
                std::vector<Card> cardsReceived;
                cardsReceived.resize(cardsOnTableCount);
                in.read(reinterpret_cast<char *>(&cardsReceived[0]), sizeof(Card) * cardsOnTableCount);
                performCHECKTurn(cardsReceived, true);
                break;
            }
            case mtgb::SERVER_CLIENTTURNPASS:{
                performPASSTurn();
                break;
            }
            default: {
                resourceStrings::print("Unexpected Packet Type Received in class clientBluff "
                                       "message type not in enum mtgb\r\n");
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
    roundTurns.clear();

    numberOfCards.find(turnSequence[whoWillTurnNext_TurnSequenceIndex])->second -= numberOfCardsTurned;
    if(turnSequence[whoWillTurnNext_TurnSequenceIndex] == myId){
        for(int i : selectedCardsIndicesFor_myCardsVectorLatest){
            myCards.find(myCardsVectorLatest[i].suit)->second.erase(myCardsVectorLatest[i].cardNumber);
        }
    }
    if(numberOfCards.find(turnSequence[whoWillTurnNext_TurnSequenceIndex])->second == 0){
        aPlayerIsMarkedForRemoval = true;
        markedPlayerForRemovalId = turnSequence[whoWillTurnNext_TurnSequenceIndex];
    }
    roundTurns.emplace_back(std::make_tuple(turnSequence[whoWillTurnNext_TurnSequenceIndex],
                                            bluffTurn(Bluff::turnType::FIRST, suitOfTheRound, numberOfCardsTurned)));
    incrementWhoWillTurnNext_TurnSequenceIndex();
    PF::setRoundTurns(roundTurns, players);
    PF::setWaitingForTurn(turnSequence[whoWillTurnNext_TurnSequenceIndex], players);
    PF::setCards(myCards);
    if(turnSequence[whoWillTurnNext_TurnSequenceIndex] == myId){
        //It should be a normal turn as it is a second turn.
        PF::promptNormalTurnAccumulate(clientOnly);
        setInputType(inputType::GB_NORMALTURNOFTHEROUND);
    }else{
        PF::promptSimpleNonTurnInputAccumulate(clientOnly);
        setInputType(inputType::GB_SIMPLENONTURNINPUT);
    }
}

void Bluff::clientBluff::performNORMALTurn(){

    cardsOnTableCount += numberOfCardsTurned;
    passTurnCount = 0;
    lastPlayerWhoTurnedId = turnSequence[whoWillTurnNext_TurnSequenceIndex];

    numberOfCards.find(turnSequence[whoWillTurnNext_TurnSequenceIndex])->second -= numberOfCardsTurned;
    if(turnSequence[whoWillTurnNext_TurnSequenceIndex] == myId){
        for(int i : selectedCardsIndicesFor_myCardsVectorLatest){
            myCards.find(myCardsVectorLatest[i].suit)->second.erase(myCardsVectorLatest[i].cardNumber);
        }
    }
    if(aPlayerIsMarkedForRemoval){
        aPlayerIsMarkedForRemoval = false;
        int index = 0;
        for(int i=0;i<turnSequence.size(); ++i){
            if(turnSequence[i] == markedPlayerForRemovalId){
                index = i;
                break;
            }
        }
        turnSequence.erase(turnSequence.begin() + index);
        numberOfCards.erase(markedPlayerForRemovalId);
        --whoWillTurnNext_TurnSequenceIndex;
        if(whoWillTurnNext_TurnSequenceIndex == -1){
            whoWillTurnNext_TurnSequenceIndex = turnSequence.size() - 1;
        }
    }
    if(numberOfCards.find(turnSequence[whoWillTurnNext_TurnSequenceIndex])->second == 0){

        aPlayerIsMarkedForRemoval = true;
        markedPlayerForRemovalId = turnSequence[whoWillTurnNext_TurnSequenceIndex];
    }

    roundTurns.emplace_back(std::make_tuple(turnSequence[whoWillTurnNext_TurnSequenceIndex],
                                                            bluffTurn(Bluff::turnType::NORMAL, numberOfCardsTurned)));
    incrementWhoWillTurnNext_TurnSequenceIndex();

    PF::setRoundTurns(roundTurns, players);
    PF::setCards(myCards);
    PF::setWaitingForTurn(turnSequence[whoWillTurnNext_TurnSequenceIndex], players);

    if(turnSequence[whoWillTurnNext_TurnSequenceIndex] == myId){
        //It should be a normal turn as it is a second turn.
        PF::promptNormalTurnAccumulate(clientOnly);
        setInputType(inputType::GB_NORMALTURNOFTHEROUND);
    }else{
        PF::promptSimpleNonTurnInputAccumulate(clientOnly);
        setInputType(inputType::GB_SIMPLENONTURNINPUT);
    }
    if(turnSequence.size() == 1){
        lobby.gameFinished();
    }
}

void Bluff::clientBluff::performCHECKTurn(std::vector<Card> cardsReceived, bool didIWrongCheckedOrBluffed){

    firstTurnOfRoundExpected = true;
    passTurnCount = 0;

    bool bluff = false;
    if(didIWrongCheckedOrBluffed){
        assert(cardsReceived.size() == cardsOnTableCount && "Why I didn't received all cards if I "
                                                            "wrongCheckedOrBluffed");

        for(int i = (cardsOnTableCount-numberOfCardsTurned); i<cardsOnTableCount; ++i){
            if(cardsReceived[i].suit != suitOfTheRound){
                bluff = true;
                break;
            }
        }
        numberOfCards.find(myId)->second += cardsReceived.size();
        for(auto & i : cardsReceived){
            myCards.find(i.suit)->second.emplace(i.cardNumber);
        }
        PF::setCards(myCards);
        if(!bluff){
            for(int i=0;i<turnSequence.size(); ++i){
                if(lastPlayerWhoTurnedId == turnSequence[i]){
                    whoWillTurnNext_TurnSequenceIndex = i;
                    break;
                }
            }
        }
    }else{
        for(auto& i: cardsReceived){
            if(i.suit != suitOfTheRound){
                bluff = true;
            }
        }
        if(bluff){
            numberOfCards.find(lastPlayerWhoTurnedId)->second += cardsOnTableCount;
        }else{
            numberOfCards.find(turnSequence[whoWillTurnNext_TurnSequenceIndex])->second += cardsOnTableCount;
            for(int i=0;i<turnSequence.size(); ++i){
                if(lastPlayerWhoTurnedId == turnSequence[i]){
                    whoWillTurnNext_TurnSequenceIndex = i;
                    break;
                }
            }
        }
    }
    cardsOnTableCount = 0;
    std::vector<Card> lastTurnedCards;
    lastTurnedCards.resize(numberOfCardsTurned);
    if(didIWrongCheckedOrBluffed){
        std::copy(cardsReceived.begin() +(cardsReceived.size() - numberOfCardsTurned), cardsReceived.end(),
                  lastTurnedCards.begin());
    }else{
        std::copy(cardsReceived.begin() , cardsReceived.end(), lastTurnedCards.begin());
    }
    roundTurns.emplace_back(std::make_tuple(turnSequence[whoWillTurnNext_TurnSequenceIndex],
                                            bluffTurn(turnType::CHECK, lastTurnedCards)));
    PF::setRoundTurns(roundTurns, players);
    PF::setWaitingForTurn(turnSequence[whoWillTurnNext_TurnSequenceIndex], players);

    if(aPlayerIsMarkedForRemoval){
        aPlayerIsMarkedForRemoval = false;
        if(!bluff){
            int index = 0;
            for(int i=0;i<turnSequence.size(); ++i){
                if(turnSequence[i] == markedPlayerForRemovalId){
                    index = i;
                    break;
                }
            }
            if(whoWillTurnNext_TurnSequenceIndex > index){
                turnSequence.erase(turnSequence.begin() + index);
                numberOfCards.erase(markedPlayerForRemovalId);
                --whoWillTurnNext_TurnSequenceIndex;
            }else{
                turnSequence.erase(turnSequence.begin() + index);
                numberOfCards.erase(markedPlayerForRemovalId);
            }
        }
    }
    if(turnSequence[whoWillTurnNext_TurnSequenceIndex] == myId){
        //It is our turn and it is first turn.
        PF::promptFirstTurnAccumulate(clientOnly);
        setInputType(inputType::GB_FIRSTTURNOFTHEROUND);
    }else{
        PF::promptSimpleNonTurnInputAccumulate(clientOnly);
        setInputType(inputType::GB_SIMPLENONTURNINPUT);
    }
    if(turnSequence.size() == 1){
        lobby.gameFinished();
    }
}

void Bluff::clientBluff::performPASSTurn(){
    ++passTurnCount;
    roundTurns.emplace_back(std::make_tuple(turnSequence[whoWillTurnNext_TurnSequenceIndex],
                                            bluffTurn(Bluff::turnType::PASS)));
    if((aPlayerIsMarkedForRemoval && passTurnCount == turnSequence.size() -1) || passTurnCount == turnSequence.size()){
        if(aPlayerIsMarkedForRemoval && passTurnCount == turnSequence.size() -1){
            aPlayerIsMarkedForRemoval = false;
            int index =-1;
            for(int i=0;i<turnSequence.size(); ++i){
                if(markedPlayerForRemovalId == turnSequence[i]){
                    index = i;
                    break;
                }
            }
            assert(index != -1);
            turnSequence.erase(turnSequence.begin() + index);
            numberOfCards.erase(markedPlayerForRemovalId);
            --whoWillTurnNext_TurnSequenceIndex;
            if(whoWillTurnNext_TurnSequenceIndex == -1){
                whoWillTurnNext_TurnSequenceIndex = turnSequence.size() - 1;
            }
        }
        firstTurnOfRoundExpected = true;
        cardsOnTableCount = 0;
    }

    incrementWhoWillTurnNext_TurnSequenceIndex();
    PF::setRoundTurns(roundTurns, players);
    PF::setWaitingForTurn(turnSequence[whoWillTurnNext_TurnSequenceIndex], players);

    if(turnSequence[whoWillTurnNext_TurnSequenceIndex] == myId){
        if(firstTurnOfRoundExpected){
            PF::promptFirstTurnAccumulate(clientOnly);
            setBaseAndInputType(this, inputType::GB_FIRSTTURNOFTHEROUND);
        }else{
            PF::promptNormalTurnAccumulate(clientOnly);
            setBaseAndInputType(this, inputType::GB_NORMALTURNOFTHEROUND);
        }
    }else{
        PF::promptSimpleNonTurnInputAccumulate(clientOnly);
        setBaseAndInputType(this, inputType::GB_SIMPLENONTURNINPUT);
    }
    if(turnSequence.size() == 1){
        lobby.gameFinished();
    }
}

void Bluff::clientBluff::input(std::string inputString, inputType inputReceivedType) {
    //This check is performed to synchronize so that a user input does not get evaluated in older context in case of
    //some event which changes the inputTypeExpected.
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
                    lobby.clientChatPtr->setBaseAndInputTypeForMESSAGESTRING();
                } else if (input == 2) {
                    //When you want to leave in case of client or close server in case of server, pass true. else if
                    //you want to exit application pass false.
                    //LEAVE || CLOSE SERVER
                    lobby.exitApplication(true);
                } else if (input == 3) {
                    //EXIT APPLICATION
                    lobby.exitApplication(false);
                } else if (input == 4) {
                    //PLAY CARDS
                    PF::promptFirstTurnOrNormalTurnSelectCards(myCards);
                    if(firstTurnOfRoundExpected){
                        setInputType(inputType::GB_PERFORMFIRSTTURNSELECTCARDS);
                    }else{
                        setInputType(inputType::GB_PERFORMNORMALTURNSELECTCARDS);
                    }
                } else if (input == 5) {
                    //CHECK
                    std::ostream& out = lobby.clientLobbySession.out;
                    out.write(reinterpret_cast<const char *>(&constants::mtcGame), sizeof(constants::mtcGame));
                    mtgb turnType = mtgb::CLIENT_TURNCHECK;
                    out.write(reinterpret_cast<char*>(&turnType), sizeof(turnType));
                    lobby.clientLobbySession.sendMessage();

                    PF::promptAndInputWaitingForCardsAccumulate(clientOnly);
                    setInputType(inputType::GB_SIMPLENONTURNINPUT);
                } else if (input == 6) {
                    //PASS
                    std::ostream& out = lobby.clientLobbySession.out;
                    out.write(reinterpret_cast<const char *>(&constants::mtcGame), sizeof(constants::mtcGame));
                    mtgb turnType = mtgb::CLIENT_TURNPASS;
                    out.write(reinterpret_cast<char*>(&turnType), sizeof(turnType));
                    lobby.clientLobbySession.sendMessage();

                    performPASSTurn();
                }
            }
        }else if (inputReceivedType == inputType::GB_PERFORMFIRSTTURNSELECTCARDS) {
            if(inputString.empty()){
                PF::promptFirstTurnAccumulate(clientOnly);
                setInputType(inputType::GB_FIRSTTURNOFTHEROUND);
            }
            else if(inputHelperInCardsSelection(inputString, inputType::GB_FIRSTTURNOFTHEROUND)){
                PF::promptFirsTurnSelectDeckSuitAccumulate();
                setInputType(inputType::GB_PERFORMFIRSTTURNSELECTDECKSUITTYPE);
            }else{
                PF::promptFirstTurnAccumulate(clientOnly);
                setInputType(inputType::GB_FIRSTTURNOFTHEROUND);
                resourceStrings::print(inputString + " is an invalid input\r\n");
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
                std::ostream& out = lobby.clientLobbySession.out;
                out.write(reinterpret_cast<const char *>(&constants::mtcGame), sizeof(constants::mtcGame));
                out.write(reinterpret_cast<char*>(&suit), sizeof(suit));
                int sz = selectedCardsIndicesFor_myCardsVectorLatest.size();
                out.write(reinterpret_cast<char*>(&sz), sizeof(sz));
                for(int i=0;i<selectedCardsIndicesFor_myCardsVectorLatest.size(); ++i){
                    int j = selectedCardsIndicesFor_myCardsVectorLatest[i];
                    out.write(reinterpret_cast<char *>(&myCardsVectorLatest[j]), sizeof(myCardsVectorLatest[j]));
                }
                lobby.clientLobbySession.sendMessage();

                suitOfTheRound = suit;
                numberOfCardsTurned = sz;
                performFIRSTTurn();

            }else{
                PF::promptFirstTurnAccumulate(clientOnly);
                setInputType(inputType::GB_FIRSTTURNOFTHEROUND);
            }
        }else if(inputReceivedType == inputType::GB_PERFORMNORMALTURNSELECTCARDS){
            if(inputString.empty()){
                PF::promptNormalTurnAccumulate(clientOnly);
                setInputType(inputType::GB_NORMALTURNOFTHEROUND);
            }
            else if(inputHelperInCardsSelection(inputString, inputType::GB_NORMALTURNOFTHEROUND)){
               //Here we have card numbers and we have to perform turn.
                std::ostream& out = lobby.clientLobbySession.out;
                out.write(reinterpret_cast<const char *>(&constants::mtcGame), sizeof(constants::mtcGame));
                auto turnType = mtgb::CLIENT_TURNNORMAL;
                out.write(reinterpret_cast<char*>(&turnType), sizeof(turnType));
                int sz = selectedCardsIndicesFor_myCardsVectorLatest.size();
                out.write(reinterpret_cast<char*>(&sz), sizeof(sz));
                for(int i=0;i<selectedCardsIndicesFor_myCardsVectorLatest.size(); ++i){
                    int j = selectedCardsIndicesFor_myCardsVectorLatest[i];
                    out.write(reinterpret_cast<char *>(&myCardsVectorLatest[j]), sizeof(myCardsVectorLatest[j]));
                }
                lobby.clientLobbySession.sendMessage();

                numberOfCardsTurned = sz;
                performNORMALTurn();
            }else{
                PF::promptNormalTurnAccumulate(clientOnly);
                setInputType(inputType::GB_NORMALTURNOFTHEROUND);
                resourceStrings::print(inputString + " is an invalid input\r\n");
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
        bool  success = constants::inputHelper(buf, 1, myCardsVectorLatest.size(), inputtype, inputtype,
        input);
        if(!success || input<1 || input > myCardsVectorLatest.size()){
            return false;
        }
        --input;
        auto it = std::find(selectedCardsIndicesFor_myCardsVectorLatest.begin(),
                            selectedCardsIndicesFor_myCardsVectorLatest.end(), input);
        if(it != selectedCardsIndicesFor_myCardsVectorLatest.end()){
            //it means input is already present.
            return false;
        }
        selectedCardsIndicesFor_myCardsVectorLatest.emplace_back(input);
    }
    if(selectedCardsIndicesFor_myCardsVectorLatest.empty()){
        return false;
    }
    return true;
}

void Bluff::clientBluff::setBaseAndInputTypeFromClientLobby(){
    if(turnSequence[whoWillTurnNext_TurnSequenceIndex] == myId){ //If my turn
        if(firstTurnOfRoundExpected){ //If first turn expected
            PF::promptFirstTurnAccumulate(clientOnly);
            setBaseAndInputType(this, inputType::GB_FIRSTTURNOFTHEROUND);
        }else{
            PF::promptNormalTurnAccumulate(clientOnly);
            setBaseAndInputType(this, inputType::GB_NORMALTURNOFTHEROUND);
        }
    }else{
        PF::promptSimpleNonTurnInputAccumulate(clientOnly);
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

Bluff::bluffTurn::bluffTurn(turnType checkTurn_, std::vector<Card> checkTurn_LastTurnCards_):
turn(checkTurn_), checkTurn_LastTurnCards(checkTurn_LastTurnCards_) {

}
