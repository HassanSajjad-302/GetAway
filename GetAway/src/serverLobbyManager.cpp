#include <random>
#include <serverLobbyManager.hpp>
#include <utility>
#include <cassert>
#include "constants.h"
serverLobbyManager::
serverLobbyManager(std::shared_ptr<serverListener> serverlistener_): serverlistener(std::move(serverlistener_)){
   // nextManager = std::make_shared<serverGameManager>
}

int
serverLobbyManager::
join(std::shared_ptr<session<serverLobbyManager, true>> lobbySession)
{
    lobbySession->receiveMessage();
    auto tup = std::tuple(playerNameAdvanced, std::move(lobbySession));
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
        //TODO
        //max players allowed will be 10, because many invariants will get broken otherwise
        net::post(std::get<1>(gameData.begin()->second)->sock.get_executor(), [self = this](){self->startGame();});
        //gameTimer.async_wait([self= this](errorCode ec){
            //self->startGame();
       // });
    }
    return id;
}

void
serverLobbyManager::
leave(int id)
{
    sendPLAYERLEFTToAllExceptOne(id);
    std::cout << "Player Left: " << std::get<0>(gameData.find(id)->second) << std::endl;
    gameData.erase(gameData.find(id));
}

void serverLobbyManager::packetReceivedFromNetwork(std::istream &in, int receivedPacketSize, int sessionId){
    lobbyMessageType messageTypeReceived;
    //STEP 1;
    in.read(reinterpret_cast<char*>(&messageTypeReceived), sizeof(lobbyMessageType));
    if(messageTypeReceived == lobbyMessageType::CHATMESSAGE){
        int senderId;
        //TODO
        //This is an extra read, delete it.
        //STEP 2;
        in.read(reinterpret_cast<char*>(&senderId), sizeof(senderId));
        assert(senderId == sessionId);
        char arr[receivedPacketSize - 4];
        //STEP 3;
        in.getline(arr, receivedPacketSize -4);
        managementCHATMESSAGEReceived(std::string(arr), sessionId);
    }
    else if(messageTypeReceived == lobbyMessageType::GAMETURNCLIENT && gameStarted){
        int index;
        if(indexGamePlayerDataFromId(sessionId, index)){
            if(gamePlayersData[index].turnExpected) {
                deckSuit suit;
                int receivedCardNumber;
                in.read(reinterpret_cast<char *>(&suit), sizeof(suit));
                in.read(reinterpret_cast<char *>(&receivedCardNumber), sizeof(receivedCardNumber));
                managementGAMETURNCLIENTReceived(sessionId, Card(suit, receivedCardNumber));
            }
        }else{
            std::cout<<"playerData with this id could not be found in the gamePlayersData. "
                       "Most likely it was removed because it's game ended. But still receieved"
                       "a GAMETURNCLIENT message from it"<<std::endl;
        }
    }
    else{
        std::cout<<"Unexpected Packet Type Received in class serverLobbyManager"<<std::endl;
    }
    std::get<1>(gameData.find(sessionId)->second)->receiveMessage();
}

void serverLobbyManager::sendPLAYERJOINEDToAllExceptOne(int excitedSessionId) {

    for(auto& player: gameData){
        if(player.first != excitedSessionId){
            auto playerSession = std::get<1>(player.second);
            std::ostream& out = playerSession->out;

            //STEP 1;
            lobbyMessageType t = lobbyMessageType::PLAYERJOINED;
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

            lobbyMessageType t = lobbyMessageType::PLAYERLEFT;
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
            lobbyMessageType t = lobbyMessageType::CHATMESSAGEID;
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
    for(auto& player: gameData){
        if(player.first != sessionId){
            auto playerSession = std::get<1>(player.second);
            std::ostream& out = playerSession->out;

            //STEP 1;
            lobbyMessageType t = lobbyMessageType::GAMETURNSERVER;
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
    lobbyMessageType t = lobbyMessageType::SELFANDSTATE;
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
    std::cout << "Player Joined: " << std::get<0>(gameData.find(excitedSessionId)->second) << std::endl;
}


void serverLobbyManager::managementCHATMESSAGEReceived(const std::string &chatMessageReceived, int excitedSessionId) {
    sendCHATMESSAGEIDToAllExceptOne(chatMessageReceived, excitedSessionId);
    std::cout<<"Message Sent To All Clients " << std::endl;
}
void serverLobbyManager::uselessWriteFunction(int id){

}
void serverLobbyManager::setPlayerNameAdvanced(std::string advancedPlayerName) {
    playerNameAdvanced = std::move(advancedPlayerName);
}

//Game Functions
void serverLobbyManager::startGame(){
    //At this point playerGameCards is ready to be distributed.
    errorCode ec;
    serverlistener->sock.shutdown(tcp::socket::shutdown_both, ec);
    serverlistener->sock.close();
    initializeGame();
    doFirstTurnOfFirstRound();
    gameStarted = true;
    firstRound = true;
    if(roundTurns.size() == gamePlayersData.size()){

    }
}

void serverLobbyManager::initializeGame(){
    auto rd = std::random_device {};
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
    for(int i=0; i<playersIdList.size(); ++i){
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
    std::vector<std::tuple<int, Card>>turnAlreadyDetermined; //id and cardValue
    constants::initializeCards(flushedCards);
    for(auto& player: gamePlayersData){
        if(player.cards.find(deckSuit::SPADE)->second.find(0) !=player.cards.find(deckSuit::SPADE)->second.end()){
            turnAlreadyDetermined.emplace_back(player.id, Card(deckSuit::SPADE, 0));
            player.cards.find(deckSuit::SPADE)->second.erase(0);
            flushedCards.find(deckSuit::SPADE)->second.emplace(0);
            continue;
        }
        if(player.cards.find(deckSuit::SPADE)->second.size() == 1){
            int cardNumber = *player.cards.find(deckSuit::SPADE)->second.begin();
            turnAlreadyDetermined.emplace_back(player.id, Card(deckSuit::SPADE, cardNumber));
            player.cards.find(deckSuit::SPADE)->second.erase(cardNumber);
            flushedCards.find(deckSuit::SPADE)->second.emplace(cardNumber);
            continue;
        }
        player.turnExpected = true;
        if(player.cards.find(deckSuit::SPADE)->second.empty()){
            player.turnTypeExpected = turnType::FIRSTROUNDANY;
        }else{
            player.turnTypeExpected = turnType::FIRSTROUNDSPADE;
        }
    }

    //first turn message sending
    for(int i=0; i<gamePlayersData.size(); ++i){
        auto& player = gamePlayersData[i];
        int currentIndexGamePlayersData = i;

        auto playerSession = std::get<1>(gameData.find(gamePlayersData[i].id)->second);
        std::ostream& out = playerSession->out;

        lobbyMessageType t = lobbyMessageType::GAMEFIRSTTURNSERVER;
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

        //STAGE 3;
        int turnAlreadyDeterminedSize = turnAlreadyDetermined.size();
        //STEP 6;
        out.write(reinterpret_cast<char*>(&turnAlreadyDeterminedSize), sizeof(turnAlreadyDeterminedSize));
        for(auto& alreadyDetermined: turnAlreadyDetermined){
            int id = std::get<0>(alreadyDetermined);
            deckSuit suit = std::get<1>(alreadyDetermined).suit;
            int cardNumber = std::get<1>(alreadyDetermined).cardNumber;
            //STEP 7;
            out.write(reinterpret_cast<char*>(&id), sizeof(id));
            //STEP 8;
            out.write(reinterpret_cast<char*>(&suit), sizeof(suit));
            //STEP 9;
            out.write(reinterpret_cast<char*>(&cardNumber), sizeof(cardNumber));
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

void serverLobbyManager::managementGAMETURNCLIENTReceived(int sessionId, Card cardReceived) {
    //iterating over gamePlayerData
    auto turnReceivedPlayer = std::find_if(gamePlayersData.begin(), gamePlayersData.end(),
                                               [sessionId](playerData& p){
        return p.id == sessionId;
    });//Be Careful Of turnReceivedPlayer Invalidation during the function
    assert(turnReceivedPlayer != gamePlayersData.end() && "no gamePlayerData id matches with sessionId");
    auto iterSuit = turnReceivedPlayer->cards.find(cardReceived.suit);
    if(iterSuit == turnReceivedPlayer->cards.end()) {
        std::cout << "A Client Turn Received But Not Acted Upon. Card Number not in card list user.\r\n";
        return;
    }
    auto iter = iterSuit->second.find(cardReceived.cardNumber);
    if(iter == iterSuit->second.end()) {
        std::cout << "A Client Turn Received But Not Acted Upon. Card Number not in card list user.\r\n";
        return;
    }
    if(firstRound){
        if(turnReceivedPlayer->turnTypeExpected == turnType::FIRSTROUNDANY){
            doTurnReceivedOfFirstRound(turnReceivedPlayer, cardReceived);//paste that code after this line
        }else{
            if(cardReceived.suit != deckSuit::SPADE) {
                std::cout << "A Turn Received But Not Acted Upon. In First Turn Client"
                             "Turned Other Card When It Had The Spade\r\n";
                return;
            }
            doTurnReceivedOfFirstRound(turnReceivedPlayer, cardReceived);//paste that code after this line
        }

    }else{
        Turn(turnReceivedPlayer, cardReceived);
    }
}

void
serverLobbyManager::doTurnReceivedOfFirstRound(
        std::vector<playerData>::iterator turnReceivedPlayer, Card cardReceived) {
    turnCardNumberOfGamePlayerIterator(turnReceivedPlayer, cardReceived);
    roundTurns.emplace_back(turnReceivedPlayer->id, cardReceived);
    if(roundTurns.size() != gamePlayersData.size()) {
        return;
    }
    roundTurns.clear();
    firstRound = false;
    for(auto t: roundTurns){
        if(std::get<1>(t).suit == deckSuit::SPADE && std::get<1>(t).cardNumber == 0){
            //This is ace of spade
            for(auto it = gamePlayersData.begin(); it!=gamePlayersData.end(); ++it){
                if(it->id == std::get<0>(t)){
                    newRoundTurn(it);
                    break;
                }
            }
            break;
        }
    }
}

void serverLobbyManager::newRoundTurn(std::vector<playerData>::iterator currentGamePlayer){
    if(constants::cardsCount(currentGamePlayer->cards) == 1){
        currentGamePlayer->turnTypeExpected = turnType::ROUNDFIRSTTURN;
        for(auto &cardPair: currentGamePlayer->cards){
            if(!cardPair.second.empty()){
                suitOfTheRound = static_cast<deckSuit>(cardPair.first);
                Turn(currentGamePlayer, Card(suitOfTheRound, *cardPair.second.begin()));
            }
        }
    }else{
        currentGamePlayer->turnTypeExpected = turnType::ROUNDFIRSTTURN;
        currentGamePlayer->turnExpected = true;
    }
}

void serverLobbyManager::Turn(std::vector<playerData>::iterator currentTurnPlayer, Card card){
    if(currentTurnPlayer->turnTypeExpected == turnType::ROUNDFIRSTTURN){ //First Turn Of Round Was Expected
        performFirstOrMiddleTurn(currentTurnPlayer, card, true);
    }else if (currentTurnPlayer->turnTypeExpected == turnType::ROUNDMIDDLETURN){
        performFirstOrMiddleTurn(currentTurnPlayer, card, false);
    }else if(currentTurnPlayer->turnTypeExpected == turnType::ROUNDLASTTURN){
        performLastOrThullaTurn(currentTurnPlayer, card, true);
    }else if(currentTurnPlayer->turnTypeExpected == turnType::THULLA){//A Thulla Was Expected
        performLastOrThullaTurn(currentTurnPlayer, card, true);
    }
}

void serverLobbyManager::performFirstOrMiddleTurn(
        std::vector<playerData>::iterator currentTurnPlayer, Card card, bool firstTurn){


    std::vector<playerData>::iterator nextGamePlayerIterator;
    if(currentTurnPlayer == gamePlayersData.end()){
        nextGamePlayerIterator = gamePlayersData.begin();
    }
    nextGamePlayerIterator = ++currentTurnPlayer;
    assert(!nextGamePlayerIterator->cards.empty() && "gamePlayerData[turnIndex] is empty."
                                                     "An invariant is broken");

    if(firstTurn){
        assert(roundTurns.empty() && "If it is first turn then roundTurns should be empty");
        //The Round Was Expected
        suitOfTheRound = card.suit;
    }else{
        assert(!roundTurns.empty() && "If it is not first turn then roundTurns should not be empty");
        if(card.suit != suitOfTheRound){
            std::cout<<"A Turn Received But Not Acted Upon Because Card is not of the required suit"<<std::endl;
        }
    }

    turnCardNumberOfGamePlayerIterator(currentTurnPlayer, card);
    roundTurns.emplace_back(currentTurnPlayer->id, card);


    if(nextGamePlayerIterator->cards.find(suitOfTheRound)->second.empty()) {
        nextGamePlayerIterator->turnTypeExpected = turnType::THULLA;
        if (constants::cardsCount(nextGamePlayerIterator->cards) == 1) {
            for(auto &cardPair: nextGamePlayerIterator->cards){
                if(!cardPair.second.empty()){
                    Turn(nextGamePlayerIterator, Card(cardPair.first, *cardPair.second.begin()));
                }
            }
        } else {
            currentTurnPlayer->turnExpected = true;
        }
    }else if(nextGamePlayerIterator->cards.find(suitOfTheRound)->second.size() == 1){
        if(roundTurns.size() == gamePlayersData.size() -1){
            nextGamePlayerIterator->turnTypeExpected = turnType::ROUNDLASTTURN;
        }else{
            nextGamePlayerIterator->turnTypeExpected = turnType::ROUNDMIDDLETURN;
        }
        Turn(nextGamePlayerIterator, Card(suitOfTheRound,
             *nextGamePlayerIterator->cards.find(suitOfTheRound)->second.begin()));
    }else{
        if(roundTurns.size() == gamePlayersData.size() -1){
            nextGamePlayerIterator->turnTypeExpected = turnType::ROUNDLASTTURN;
        }else{
            nextGamePlayerIterator->turnTypeExpected = turnType::ROUNDMIDDLETURN;
        }
        nextGamePlayerIterator->turnExpected = true;
    }
}

void serverLobbyManager::performLastOrThullaTurn(
        std::vector<playerData>::iterator currentTurnPlayer, Card card, bool lastTurn){
    std::vector<playerData>::iterator roundKing;
    if(lastTurn){
        if(card.suit != suitOfTheRound){
            std::cout<<"A Turn Received But Not Acted Upon Because Card is not of the required suit"<<std::endl;
            return;
        }
        roundKing = roundKingGamePlayerDataIterator();
        for(auto cn: roundTurns){
            flushedCards.find(std::get<1>(cn).suit)->second.emplace(std::get<1>(cn).cardNumber);
        }
        flushedCards.find(card.suit)->second.emplace(card.cardNumber);
    }else{
        roundKing = roundKingGamePlayerDataIterator();
        for(auto cn: roundTurns){
            roundKing->cards.find(std::get<1>(cn).suit)->second.emplace(std::get<1>(cn).cardNumber);
        }
        roundKing->cards.find(card.suit)->second.emplace(card.cardNumber);
    }
    turnCardNumberOfGamePlayerIterator(currentTurnPlayer, card);

    gamePlayersData.erase(std::remove_if(gamePlayersData.begin(), gamePlayersData.end(),
                                         [](playerData& s){
                                             return s.cards.empty();
                                         }), gamePlayersData.end());

    if(gamePlayersData.empty()){
        //match drawn
        //exit
        //TODO
        return;
    }
    if(gamePlayersData.size() == 1){
        //That id left player has lost
        //exit
        //TODO
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
        assert((cardNumber >= 0 && cardNumber < constants::SUITSIZE) && "Card-Number not in range");
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
    throw(std::logic_error("HighestCardHolder Id not present in gamePlayerData"));
}

bool serverLobbyManager::indexGamePlayerDataFromId(int id, int& index){
    for(int i=0; i < gamePlayersData.size(); ++i){
        if(gamePlayersData[i].id == id){
            index = i;
            return true;
        }
    }
    return false;
}
