#include <random>
#include <serverLobbyManager.hpp>
#include <utility>
#include <cassert>
#include "serverHome.hpp"
#include "constants.h"
#include "serverPF.hpp"
#include "resourceStrings.hpp"
#include "sati.hpp"
serverLobbyManager::
serverLobbyManager(std::shared_ptr<serverListener> serverlistener_, asio::io_context& io_):
serverlistener(std::move(serverlistener_)), io{io_}{
    // nextManager = std::make_shared<serverGameManager>
}

int
serverLobbyManager::
join(std::shared_ptr<session<serverLobbyManager, true>> lobbySession)
{
    lobbySession->receiveMessage();
    std::tuple<std::string, std::shared_ptr<session<serverLobbyManager, true>>> tup(playerNameAdvanced, std::move(lobbySession));
    int id;
    if(gameData.empty())
    {
        id = 0;
        gameData.emplace(id,tup);
        playerNameFinal = std::move(playerNameAdvanced);
    }
    else{
        id = gameData.rbegin()->first + 1;
        auto pair = gameData.emplace(id,tup);
        playerNameFinal = std::move(playerNameAdvanced);
        //TODO
        //Currently, There is no policy for handling players with different Names.
        //Because of having same Id, I currently don't care. Though this is the only
        //reason server resends the playerName.
    }
    //Tell EveryOne SomeOne has Joined In
    managementJoin(id);

    if(gameData.size() == 2){
        sati::getInstance()->setBase(this, appState::LOBBY);
        serverPF::setLobbyMainTwoOrMorePlayers();
        setInputType(inputType::SERVERLOBBYTWOORMOREPLAYERS);
    }
    return id;
}

void
serverLobbyManager::
leave(int id)
{
    if(!gameStarted){
        sendPLAYERLEFTToAllExceptOne(id);
        resourceStrings::print("Player Left: " + std::get<0>(gameData.find(id)->second) + "\r\n");
        gameData.erase(gameData.find(id));
        if(gameData.size() == 1){
            goBackToServerListener();
        }
    }else{
        resourceStrings::print("Player Left During The Game\r\n");
        exit(-1);
    }

}

void serverLobbyManager::packetReceivedFromNetwork(std::istream &in, int receivedPacketSize, int sessionId){
    messageType messageTypeReceived;
    //STEP 1;
    in.read(reinterpret_cast<char*>(&messageTypeReceived), sizeof(messageType));
    if(messageTypeReceived == messageType::CHATMESSAGE){
        int senderId;
        //TODO
        //This is an extra read, delete it.
        //STEP 2;
        in.read(reinterpret_cast<char*>(&senderId), sizeof(senderId));
        assert(senderId == sessionId);
#if defined(_WIN32) || defined(_WIN64)
        char* arr = new char[receivedPacketSize - 4];
#endif
#ifdef __linux__
		char arr[receivedPacketSize - 4];
#endif
        //STEP 3;
        in.getline(arr, receivedPacketSize -4);
        managementCHATMESSAGEReceived(std::string(arr), sessionId);
#if defined(_WIN32) || defined(_WIN64)
        delete[] arr;
#endif
    }
    else if(messageTypeReceived == messageType::GAMETURNCLIENT && gameStarted){
        constants::Log("GAMETURNCLIENT received");
        int index;
        if(indexGamePlayerDataFromId(sessionId, index)){
            if(gamePlayersData[index].turnExpected) {
                constants::Log("Turn Expected from this GameTurnClient");
                deckSuit suit;
                int receivedCardNumber;
                in.read(reinterpret_cast<char *>(&suit), sizeof(suit));
                in.read(reinterpret_cast<char *>(&receivedCardNumber), sizeof(receivedCardNumber));
                managementGAMETURNCLIENTReceived(sessionId, Card(suit, receivedCardNumber));
            }else{
                constants::Log("Turn Not Expected From This Client");
            }
        }else{
            resourceStrings::print("playerData with this id could not be found in the gamePlayersData. "
                                   "Most likely it was removed because it's game ended. But still receieved"
                                   "a GAMETURNCLIENT message from it\r\n");
        }
    }
    else{
        resourceStrings::print("Unexpected Packet Type Received in class serverLobbyManager\r\n");
    }
    std::get<1>(gameData.find(sessionId)->second)->receiveMessage();
}

void serverLobbyManager::sendPLAYERJOINEDToAllExceptOne(int excitedSessionId) {

    for(auto& player: gameData){
        if(player.first != excitedSessionId){
            auto playerSession = std::get<1>(player.second);
            std::ostream& out = playerSession->out;

            //STEP 1;
            messageType t = messageType::PLAYERJOINED;
            out.write(reinterpret_cast<char*>(&t), sizeof(t));
            //STEP 2;
            auto mapPtr = gameData.find(excitedSessionId);
            int id = mapPtr->first;
            out.write(reinterpret_cast<char*>(&id), sizeof(id));
            //STEP 3;
            out << std::get<0>(mapPtr->second) << std::endl;

            playerSession->sendMessage(&serverLobbyManager::uselessWriteFunction);
        }
    }
}

void serverLobbyManager::sendPLAYERLEFTToAllExceptOne(int excitedSessionId) {
    for(auto& player: gameData){
        if(player.first != excitedSessionId){
            auto playerSession = std::get<1>(player.second);
            std::ostream& out = playerSession->out;

            messageType t = messageType::PLAYERLEFT;
            //STEP 1;
            out.write(reinterpret_cast<char*>(&t), sizeof(t));
            auto mapPtr = gameData.find(excitedSessionId);
            int id = mapPtr->first;
            //STEP 2;
            out.write(reinterpret_cast<char*>(&id), sizeof(id));

            playerSession->sendMessage(&serverLobbyManager::uselessWriteFunction);
        }
    }
}

void serverLobbyManager::sendCHATMESSAGEIDToAllExceptOne(const std::string &chatMessageReceived, int excitedSessionId) {

    for(auto& player: gameData){
        if(player.first != excitedSessionId){
            auto playerSession = std::get<1>(player.second);
            std::ostream& out = playerSession->out;

            //STEP 1;
            messageType t = messageType::CHATMESSAGEID;
            out.write(reinterpret_cast<char*>(&t), sizeof(t));
            //STEP 2;
            out.write(reinterpret_cast<char *>(&excitedSessionId), sizeof(excitedSessionId));
            //STEP 3;
            out << chatMessageReceived << std::endl;

            playerSession->sendMessage(&serverLobbyManager::uselessWriteFunction);
        }
    }
}

void serverLobbyManager::sendGAMETURNSERVERTOAllExceptOne(int sessionId, Card card) {
    constants::Log("Sending GameTurnServerToAllExceptOne to all. Called By Session-Id {}", sessionId);
    constants::Log("Sending Card {} {}", deckSuitValue::displaySuitType[(int) card.suit],
                 deckSuitValue::displayCards[card.cardNumber]);
    for(auto& player: gameData){
        if(player.first != sessionId){
            auto playerSession = std::get<1>(player.second);
            std::ostream& out = playerSession->out;

            //STEP 1;
            messageType t = messageType::GAMETURNSERVER;
            out.write(reinterpret_cast<char*>(&t), sizeof(t));
            //STEP 2;
            out.write(reinterpret_cast<char *>(&sessionId), sizeof(sessionId));
            //STEP 3;
            out.write(reinterpret_cast<char *>(&card.suit), sizeof(card.suit));
            //STEP 4;
            out.write(reinterpret_cast<char*>(&card.cardNumber), sizeof(card.cardNumber));

            playerSession->sendMessage(&serverLobbyManager::uselessWriteFunction);
        }
    }
}

//excitedSessionId is the one to send state to and update of it to remaining
void serverLobbyManager::managementJoin(int excitedSessionId) {

    auto playerSession = std::get<1>(gameData.find(excitedSessionId)->second);
    std::ostream& out = playerSession->out;

    //STEP 1;
    messageType t = messageType::SELFANDSTATE;
    out.write(reinterpret_cast<char*>(&t), sizeof(t));
    //STEP 2;
    out.write(reinterpret_cast<char *>(&excitedSessionId), sizeof(excitedSessionId));
    //STEP 3;
    out << playerNameFinal << std::endl;
    //STEP 4;
    int size = gameData.size();
    out.write(reinterpret_cast<char *>(&size), sizeof(size));
    for(auto& gamePlayer : gameData){
        //STEP 5;
        int id = gamePlayer.first;
        out.write(reinterpret_cast<char *>(&id), sizeof(id));
        //STEP 6;
        out << std::get<0>(gamePlayer.second) << std::endl;
    }
    playerSession->sendMessage(&serverLobbyManager::uselessWriteFunction);

    sendPLAYERJOINEDToAllExceptOne(excitedSessionId);
    resourceStrings::print("Player Joined: " + std::get<0>(gameData.find(excitedSessionId)->second) + "\r\n");

}


void serverLobbyManager::managementCHATMESSAGEReceived(const std::string &chatMessageReceived, int excitedSessionId) {
    sendCHATMESSAGEIDToAllExceptOne(chatMessageReceived, excitedSessionId);
    resourceStrings::print("Message Sent To All Clients\r\n");
}
void serverLobbyManager::uselessWriteFunction(int id){

}
void serverLobbyManager::setPlayerNameAdvanced(std::string advancedPlayerName) {
    playerNameAdvanced = std::move(advancedPlayerName);
}

//Game Functions
void serverLobbyManager::closeLobbySocketsAndStartGame(){
    //At this point playerGameCards is ready to be distributed.
    errorCode ec;
    serverlistener->shutdownAcceptorAndProbe();
    initializeGame();
    doFirstTurnOfFirstRound();
    gameStarted = true;
    firstRound = true;
}

void serverLobbyManager::initializeGame(){
    std::random_device rd{};
    //todo
    //providing a same value to random engine for testing
    auto rng = std::default_random_engine { 1 };

    int numberOfPlayersWithExtraCards = constants::DECKSIZE % gameData.size();

    std::vector<int> playersIdList;
    for(const auto& player: gameData){
        playersIdList.push_back(player.first);
    }
    std::shuffle(playersIdList.begin(), playersIdList.end(), rng);

    std::vector<int> playerWithExtraCardIdsList;
    playerWithExtraCardIdsList.reserve(numberOfPlayersWithExtraCards);
    for(int i=0;i<numberOfPlayersWithExtraCards;++i){
        playerWithExtraCardIdsList.push_back(playersIdList[i]);
    }
    std::shuffle(playerWithExtraCardIdsList.begin(), playerWithExtraCardIdsList.end(), rng);

    int cards[constants::DECKSIZE];
    for(int i=0;i<constants::DECKSIZE;++i){
        cards[i] = i;
    }
    std::shuffle(cards, cards + constants::DECKSIZE, rng);

    int normalNumberOfCards = constants::DECKSIZE/gameData.size();

    int cardsCount = 0;
    for(u_int i=0; i<playersIdList.size(); ++i){
        auto p = playerData(playersIdList[i]);
        if(std::find(playerWithExtraCardIdsList.begin(),playerWithExtraCardIdsList.end(),gameData.find(i)->first) !=
           playerWithExtraCardIdsList.end()){
            for(int j=0; j<normalNumberOfCards+1; ++j){
                assert(cards[cardsCount] >= 0 && cards[cardsCount] < constants::DECKSIZE && "Card Id is out-of-range");
                auto suit = static_cast<deckSuit>(cards[cardsCount] / constants::SUITSIZE);
                int cardNumber = cards[cardsCount] % constants::SUITSIZE;
                p.insertCard(Card(suit, cardNumber));
                ++cardsCount;
            }
        }else{
            for(int j =0; j<normalNumberOfCards; ++j){
                assert(cards[cardsCount] >= 0 && cards[cardsCount] < constants::DECKSIZE && "Card Id is out-of-range");
                auto suit = static_cast<deckSuit>(cards[cardsCount] / constants::SUITSIZE);
                int cardNumber = cards[cardsCount] % constants::SUITSIZE;
                p.insertCard(Card(suit, cardNumber));
                ++cardsCount;
            }
        }
        gamePlayersData.emplace_back(std::move(p));
    }

#ifndef NDEBUG
    int size = 0;
    for(auto& p: gamePlayersData){
        size += constants::cardsCount(p.cards);
    }
    assert(size == constants::DECKSIZE && "Sum Of Cards distributed not equal to constants::DECKSIZE");
#endif

}

//We need to check for in first turn whose players turn can be performed by the computer. This will be checked by
//checking the integer value of 26 in the map of playerGameCards.
//We will then make and send another vector<int> which will have the list of those ids whose turn is also determined.

//All user card hand is not sent. But user can determine the full hand by seeing it's turn in
//turnAlreadyDetermined.
void serverLobbyManager::doFirstTurnOfFirstRound(){
    //check for auto first turn possibilities
    constants::initializeCards(flushedCards);
    for(auto& player: gamePlayersData){
        player.turnExpected = true;
        if(player.cards.find(deckSuit::SPADE)->second.empty()){
            player.turnTypeExpected = turnType::FIRSTROUNDANY;
        }else{
            player.turnTypeExpected = turnType::FIRSTROUNDSPADE;
        }
    }

    //first turn message sending
    for(u_int i=0; i<gamePlayersData.size(); ++i){
        auto& player = gamePlayersData[i];
        int currentIndexGamePlayersData = i;

        auto playerSession = std::get<1>(gameData.find(gamePlayersData[i].id)->second);
        std::ostream& out = playerSession->out;

        messageType t = messageType::GAMEFIRSTTURNSERVER;
        //STEP 1;
        out.write(reinterpret_cast<char*>(&t), sizeof(t));
        auto &p = gamePlayersData[currentIndexGamePlayersData];
        int handSize = constants::cardsCount(p.cards);
        //STEP 2;
        out.write(reinterpret_cast<char*>(&handSize),sizeof(handSize));
        for(auto& cardPair: p.cards){
            for(auto c: cardPair.second) {
                //STEP 3; //deckSuit
                out.write(reinterpret_cast<const char *>(&cardPair.first), sizeof(cardPair.first));
                //STEP 4;
                out.write(reinterpret_cast<char *>(&c), sizeof(c));
            }
        }

        //STAGE 2;
        for(auto& turnSequenceId: gamePlayersData){
            int sequenceId = turnSequenceId.id;
            //STEP 5; //Turn Sequence
            out.write(reinterpret_cast<char*>(&sequenceId), sizeof(sequenceId));
        }

        for(auto& gp: gamePlayersData){
            int gpId = gp.id;
            //STEP 10;
            out.write(reinterpret_cast<char*>(&gpId), sizeof(gpId));
            int gpCardsSize = constants::cardsCount(gp.cards);
            //STEP 11;
            out.write(reinterpret_cast<char*>(&gpCardsSize), sizeof(gpCardsSize));
        }
        playerSession->sendMessage(&serverLobbyManager::uselessWriteFunction);
    }
}

//invariants
//In first turn it is calculated where turn is possible. If turn is possible that card-number and player-id is added
//in round-turns.
//That card is flushed and not sent to the client.
//Turn is not expected from these players.
//If a turn is being received in first turn then it is only received when turn could not be auto determined by the
//server.
//gamePlayerData is randomly arranged at initialization. So it's sequence is turnSequence.

//In first turn it is also set whether suit of the card received should be of type deckSuit::ANY or deckSuit::SPADE
//whether badranga is coming or the spade card because client had more than one spade card.
//Following function should not be called if there is one player left i.e gamePlayersData.size() >1

#ifndef NDEBUG
void serverLobbyManager::checkForCardsCount(){
    int cardsCount = 0;
    cardsCount += constants::cardsCount(flushedCards);
    cardsCount += roundTurns.size();
    for(auto& p: gamePlayersData){
        cardsCount += constants::cardsCount(p.cards);
    }
    if(cardsCount != constants::DECKSIZE){
        resourceStrings::print("Flushed Cards " + std::to_string(constants::cardsCount(flushedCards)) + "\r\n");
        resourceStrings::print("Round Turns " + std::to_string(roundTurns.size()) + "\r\n");
        for(auto& p: gamePlayersData){
            resourceStrings::print("Player Id " + std::to_string(p.id) + "  Player Cards Size " +
            std::to_string(constants::cardsCount(p.cards)) + "\r\n");
        }
    }
    assert(cardsCount == constants::DECKSIZE && "Card-Count not equal to 52 error");

}
#endif
void serverLobbyManager::managementGAMETURNCLIENTReceived(int sessionId, Card cardReceived) {
#ifndef NDEBUG
    checkForCardsCount();
#endif

    constants::Log("Game Turn Client Received From{}", std::get<0>(gameData.find(sessionId)->second));
    constants::Log("Card Received Is {} {}", deckSuitValue::displaySuitType[(int)cardReceived.suit],
                 deckSuitValue::displayCards[cardReceived.cardNumber]);
    //iterating over gamePlayerData
    auto turnReceivedPlayer = std::find_if(gamePlayersData.begin(), gamePlayersData.end(),
                                               [sessionId](playerData& p){
        return p.id == sessionId;
    });//Be Careful Of turnReceivedPlayer Invalidation during the function
    assert(turnReceivedPlayer != gamePlayersData.end() && "no gamePlayerData id matches with sessionId");
    auto iterSuit = turnReceivedPlayer->cards.find(cardReceived.suit);
    if(iterSuit == turnReceivedPlayer->cards.end()) {
        resourceStrings::print("A Client Turn Received But Not Acted Upon. Card Number not in card list user.\r\n");
        return;
    }
    auto iter = iterSuit->second.find(cardReceived.cardNumber);
    if(iter == iterSuit->second.end()) {
        resourceStrings::print("A Client Turn Received But Not Acted Upon. Card Number not in card list user.\r\n");
        return;
    }
    if(firstRound){
        if(turnReceivedPlayer->turnTypeExpected == turnType::FIRSTROUNDANY){
            constants::Log("First Round Any was expected from this user");
            doTurnReceivedOfFirstRound(turnReceivedPlayer, cardReceived);
        }else{
            if(cardReceived.suit != deckSuit::SPADE) {
                resourceStrings::print("A Turn Received But Not Acted Upon. In First Turn Client"
                                       "Turned Other Card When It Had The Spade\r\n");
                return;
            }
            constants::Log("First Round SPADE was expected from this user");
            doTurnReceivedOfFirstRound(turnReceivedPlayer, cardReceived);//paste that code after this line
        }

    }else{
        Turn(turnReceivedPlayer, cardReceived);
    }

#ifndef NDEBUG
    if(gameStarted) {
        checkForCardsCount();
    }
#endif
}

void
serverLobbyManager::doTurnReceivedOfFirstRound(
        std::vector<playerData>::iterator turnReceivedPlayer, Card cardReceived) {
    turnCardNumberOfGamePlayerIterator(turnReceivedPlayer, cardReceived);
    roundTurns.emplace_back(turnReceivedPlayer->id, cardReceived);
    if(roundTurns.size() != gamePlayersData.size()) {
        return;
    }
    firstRound = false;
    for(auto& rt: roundTurns){
        flushedCards.find(std::get<1>(rt).suit)->second.emplace(std::get<1>(rt).cardNumber);
    }
    for(auto t: roundTurns){
        if(std::get<1>(t).suit == deckSuit::SPADE && std::get<1>(t).cardNumber == 0){
            //This is ace of spade
            for(auto it = gamePlayersData.begin(); it!=gamePlayersData.end(); ++it){
                if(it->id == std::get<0>(t)){
                    roundTurns.clear();
                    newRoundTurn(it);
                    break;
                }
            }
            break;
        }
    }
}

void serverLobbyManager::newRoundTurn(std::vector<playerData>::iterator currentGamePlayer){
        currentGamePlayer->turnTypeExpected = turnType::ROUNDFIRSTTURN;
        currentGamePlayer->turnExpected = true;
}

void serverLobbyManager::Turn(std::vector<playerData>::iterator currentTurnPlayer, Card card){
    if(currentTurnPlayer->turnTypeExpected == turnType::ROUNDFIRSTTURN){ //First Turn Of Round Was Expected
        performFirstOrMiddleTurn(currentTurnPlayer, card, true);
    }else if (currentTurnPlayer->turnTypeExpected == turnType::ROUNDMIDDLETURN){
        performFirstOrMiddleTurn(currentTurnPlayer, card, false);
    }else if(currentTurnPlayer->turnTypeExpected == turnType::ROUNDLASTTURN){
        performLastOrThullaTurn(currentTurnPlayer, card, true);
    }else if(currentTurnPlayer->turnTypeExpected == turnType::THULLA){//A Thulla Was Expected
        performLastOrThullaTurn(currentTurnPlayer, card, false);
    }
}

void serverLobbyManager::performFirstOrMiddleTurn(
        std::vector<playerData>::iterator currentTurnPlayer, Card card, bool firstTurn){

    constants::Log("Perform First Or Middle Turn Called. firstTurn bool value is {}", std::to_string(firstTurn));

    if(firstTurn){
        assert(roundTurns.empty() && "If it is first turn then roundTurns should be empty");
        constants::Log("First Turn Of The Round Is Received. Suit Set For The Round {}",
                     deckSuitValue::displaySuitType[(int) card.suit]);
        //First Turn Was Expected
        suitOfTheRound = card.suit;
    }else{
        assert(!roundTurns.empty() && "If it is not first turn then roundTurns should not be empty");
        if(card.suit != suitOfTheRound){
            resourceStrings::print("A Turn Received But Not Acted Upon Because"
                                   " Card is not of the required suit\r\n");
        }
    }

    turnCardNumberOfGamePlayerIterator(currentTurnPlayer, card);
    roundTurns.emplace_back(currentTurnPlayer->id, card);
    currentTurnPlayer->cards.find(card.suit)->second.erase(card.cardNumber);

    std::vector<playerData>::iterator nextGamePlayerIterator;
    nextGamePlayerIterator = std::next(currentTurnPlayer);
    if(nextGamePlayerIterator == gamePlayersData.end()){
        nextGamePlayerIterator = gamePlayersData.begin();
    }
    assert(constants::cardsCount(nextGamePlayerIterator->cards) > 0 && "Card Count should be greater than one");


    if(nextGamePlayerIterator->cards.find(suitOfTheRound)->second.empty()) {
        nextGamePlayerIterator->turnTypeExpected = turnType::THULLA;
        nextGamePlayerIterator->turnExpected = true;
        constants::Log("nextGamePlayerIterator Thulla Expected. Id {}", nextGamePlayerIterator->id);
    }else{
        if(roundTurns.size() == gamePlayersData.size() -1){
            constants::Log("nextGamePlayerIterator ROUNDLASTTURN Expected. Id {}", nextGamePlayerIterator->id);
            nextGamePlayerIterator->turnTypeExpected = turnType::ROUNDLASTTURN;
        }else{
            constants::Log("nextGamePlayerIterator ROUNDMIDDLETURN Expected. Id {}", nextGamePlayerIterator->id);
            nextGamePlayerIterator->turnTypeExpected = turnType::ROUNDMIDDLETURN;
        }
        constants::Log("Waiting For Turn. Turn Expected Called");
        nextGamePlayerIterator->turnExpected = true;
    }
}

void serverLobbyManager::performLastOrThullaTurn(
        std::vector<playerData>::iterator currentTurnPlayer, Card card, bool lastTurn){
    constants::Log("Perform First Or Middle Turn Called. firstTurn bool value is {}", std::to_string(lastTurn));
    std::vector<playerData>::iterator roundKing;
    if(lastTurn){
        if(card.suit != suitOfTheRound){
            resourceStrings::print("A Turn Received But Not Acted Upon"
                                   " Because Card is not of the required suit\r\n");
            return;
        }
        roundTurns.emplace_back(currentTurnPlayer->id, card);
        roundKing = roundKingGamePlayerDataIterator();
        for(auto cn: roundTurns){
            flushedCards.find(std::get<1>(cn).suit)->second.emplace(std::get<1>(cn).cardNumber);
        }
    }else{
        roundKing = roundKingGamePlayerDataIterator();
        roundTurns.emplace_back(currentTurnPlayer->id, card);
        for(auto cn: roundTurns){
            roundKing->cards.find(std::get<1>(cn).suit)->second.emplace(std::get<1>(cn).cardNumber);
        }
    }
    turnCardNumberOfGamePlayerIterator(currentTurnPlayer, card);

    gamePlayersData.erase(std::remove_if(gamePlayersData.begin(), gamePlayersData.end(),
                                         [](playerData& s){
                                             return constants::cardsCount(s.cards) == 0;
                                         }), gamePlayersData.end());

    roundTurns.clear();
    if(gamePlayersData.empty()){
        //match drawn
        //exit Game
        gameExitFinished();
        return;
    }
    if(gamePlayersData.size() == 1){
        //That id left player has lost
        //exit Game
        gameExitFinished();
        return;
    }
    newRoundTurn(roundKing);
}

void serverLobbyManager::turnCardNumberOfGamePlayerIterator(std::vector<playerData>::iterator turnReceivedPlayer,
                                                            Card card){
    //send turn to every-one except one
    //remove the card from turnReceiverPlayer.cards
    //set TurnExpected of current = false;
    //emplace back in roundTurns

    sendGAMETURNSERVERTOAllExceptOne(turnReceivedPlayer->id, card);
    turnReceivedPlayer->cards.find(card.suit)->second.erase(card.cardNumber);
    turnReceivedPlayer->turnExpected = false;
}

std::vector<playerData>::iterator serverLobbyManager::roundKingGamePlayerDataIterator(){
    int highestCardHolderId = std::get<0>(*roundTurns.begin());
    int highestCardNumber = std::get<1>(*roundTurns.begin()).cardNumber;
    for(auto turns: roundTurns){
        int cardNumber = std::get<1>(turns).cardNumber;
        assert((cardNumber >= 0 && cardNumber < constants::SUITSIZE) && "Card-Number not in range\r\n");
        if(cardNumber == 0 ){
            highestCardHolderId = std::get<0>(turns);
            break;
        }else{
            if(cardNumber > highestCardNumber){
                highestCardNumber = cardNumber;
                highestCardHolderId = std::get<0>(turns);
            }
        }
    }
    for(auto it = gamePlayersData.begin(); it != gamePlayersData.end(); ++it){
        if(it->id == highestCardHolderId){
            return it;
        }
    }
    throw(std::logic_error("HighestCardHolder Id not present in gamePlayerData\r\n"));
}

bool serverLobbyManager::indexGamePlayerDataFromId(int id, int& index){
    for(u_int i=0; i < gamePlayersData.size(); ++i){
        if(gamePlayersData[i].id == id){
            index = i;
            return true;
        }
    }
    return false;
}

void serverLobbyManager::input(std::string inputString, inputType inputReceivedType){
    if(inputReceivedType == inputTypeExpected){
        if(inputReceivedType == inputType::SERVERLOBBYTWOORMOREPLAYERS){
            int input;
            if(constants::inputHelper(inputString, 1, 3,inputType::SERVERLOBBYTWOORMOREPLAYERS,
                                      inputType::SERVERLOBBYTWOORMOREPLAYERS, input)){
                if(input == 1){
                    //Start The Game
                    closeLobbySocketsAndStartGame();
                    setInputType(inputType::GAMEINT);
                    sati::getInstance()->setBase(this, appState::GAME);
                    serverPF::setGameMain();
                }else if(input == 2){
                    //Close Server
                    applicationExit();
                    std::make_shared<serverHome>(serverHome(io))->run();
                }else{
                    //Exit
                    applicationExit();
                }
            }
        }
        else if(inputReceivedType == inputType::GAMEINT){
            //TODO
            //Early-End-Game. input 1 for early ending game. send early ending game message to all players.
            int input;
            if(constants::inputHelper(inputString,2,3,inputType::GAMEINT,
                                      inputType::GAMEINT, input)){
                if(input == 2){
                    applicationExit();
                }else{
                    applicationExit();
                    std::make_shared<serverHome>(serverHome(io))->run();
                }
            }
            if(inputString == "1"){
                //Exit The Application Here Amid Game
                //TODO
                //Ending Is Not Good Here. It is When Game is Occuring.
            }else{
                resourceStrings::print("Wrong Input\r\n");
                setInputType(inputType::GAMEINT);
            }
        }else{
            resourceStrings::print("No Handler For This InputType\r\n");
        }
    }else{
        resourceStrings::print("Message Of Unexpected Input Type Received\r\n");
    }
}

void serverLobbyManager::goBackToServerListener(){
    serverlistener->registerForInputReceival();
}

void serverLobbyManager::setInputType(inputType type){
    sati::getInstance()->setInputType(type);
    inputTypeExpected = type;
}

void serverLobbyManager::applicationExit() {
    serverlistener->shutdown();
    //exit(EXIT_SUCCESS);
}

void serverLobbyManager::gameExitFinished(){
#ifndef NDEBUG
    checkForCardsCount();
#endif
    flushedCards.clear();

    gameStarted = false;
    gamePlayersData.clear();
    serverlistener->runAgain();

    sati::getInstance()->setBase(this, appState::LOBBY);
    serverPF::setLobbyMainTwoOrMorePlayers();
    setInputType(inputType::SERVERLOBBYTWOORMOREPLAYERS);
}

void serverLobbyManager::shutDown() {
    constants::Log("UseCount of serverlistener from serverLobbyManager {}", serverlistener.use_count());
    serverlistener.reset();
    for(auto &p: gameData){
        std::get<1>(p.second)->sock.shutdown(asio::socket_base::shutdown_both);
        std::get<1>(p.second)->sock.close();
        constants::Log("UseCount of serverLobbySession from serverLobbyManager {}", std::get<1>(p.second).use_count());
        std::get<1>(p.second).reset();
    }
}
