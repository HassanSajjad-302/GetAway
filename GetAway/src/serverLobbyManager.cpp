
#include <random>
#include <serverLobbyManager.hpp>
#include <utility>
#include <cassert>
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
   /* if(gameData.size() == 3){
        //TODO
        //max players allowed will be 10, because many invariants will get broken otherwise
        std::this_thread::sleep_for(std::chrono::seconds(3));
        net::post(std::get<1>(gameData.begin()->second)->sock.get_executor(), [self = this](){self->startGame();});
        //gameTimer.async_wait([self= this](errorCode ec){
            //self->startGame();
       // });
    }*/
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
            if(gamePlayersData[sessionId].turnExpected) {
                int receivedCardNumber;
                in.read(reinterpret_cast<char *>(&receivedCardNumber), sizeof(receivedCardNumber));
                managementGAMETURNCLIENTReceived(receivedCardNumber, sessionId);
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

void serverLobbyManager::sendGAMETURNSERVERTOAllExceptOne(int receivedCardNumber, int excitedSessionId) {
    for(auto& player: gameData){
        if(player.first != excitedSessionId){
            auto playerSession = std::get<1>(player.second);
            std::ostream& out = playerSession->out;

            //STEP 1;
            lobbyMessageType t = lobbyMessageType::GAMETURNSERVER;
            out.write(reinterpret_cast<char*>(&t), sizeof(t));
            //STEP 2;
            out.write(reinterpret_cast<char *>(&excitedSessionId), sizeof(excitedSessionId));
            //STEP 3;
            out.write(reinterpret_cast<char*>(&receivedCardNumber), sizeof(receivedCardNumber));

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
    auto rng = std::default_random_engine { rd() };

    int numberOfPlayersWithExtraCards = 52 % gameData.size();

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

    int cards[52];
    for(int i=0;i<52;++i){
        cards[i] = i;
    }
    std::shuffle(cards, cards + 52, rng);

    int normalNumberOfCards = 52/gameData.size();

    int cardsCount = 0;
    for(int i=0; i<playersIdList.size(); ++i){
        auto p = playerData(playersIdList[i]);
        if(std::find(playerWithExtraCardIdsList.begin(),playerWithExtraCardIdsList.end(),gameData.find(i)->first) !=
           playerWithExtraCardIdsList.end()){
            for(int j=0; j<normalNumberOfCards+1; ++j){
                assert(std::find(p.cards.begin(), p.cards.end(), cards[cardsCount]) == p.cards.end() &&
                       "Card Is Already Present In The List");
                assert(cards[cardsCount] >= 0 && cards[cardsCount] < 52 && "Card Id is out-of-range");
                p.cards.push_back(cards[cardsCount]);
                ++cardsCount;
            }
        }else{
            for(int j =0; j<normalNumberOfCards; ++j){
                assert(std::find(p.cards.begin(), p.cards.end(), cards[cardsCount]) == p.cards.end() &&
                       "Card Is Already Present In The List");
                assert(cards[cardsCount] >= 0 && cards[cardsCount] < 52 && "Card Id is out-of-range");
                p.cards.push_back(cards[cardsCount]);
                ++cardsCount;
            }
        }
        gamePlayersData.emplace_back(std::move(p));
    }

#ifndef NDEBUG
    int size = 0;
    for(auto& p: gamePlayersData){
        size += p.cards.size();
    }
    assert(size == 52 && "Sum Of Cards distributed not equal to 52");
#endif

}

//We need to check for in first turn whose players turn can be performed by the computer. This will be checked by
//checking the integer value of 26 in the map of playerGameCards.
//We will then make and send another vector<int> which will have the list of those ids whose turn is also determined.

//All user card hand is not sent. But user can determine the full hand by seeing it's turn in
//turnAlreadyDetermined.
void serverLobbyManager::doFirstTurnOfFirstRound(){
    //check for auto first turn possibilities
    std::vector<std::tuple<int, int>>turnAlreadyDetermined; //id and cardValue
    for(auto& player: gamePlayersData){
        int spadeCardsCount = 0;
        int spadeCardValue;
        for(auto& cardNumber: player.cards){
            if(cardNumber == 26){ // 26 is calculated by cardNumber/13 == suitValues::spade
                turnAlreadyDetermined.emplace_back(player.id, cardNumber);
                player.cards.remove(cardNumber);
                flushedCards.emplace_back(cardNumber);
                break;
            }
            if((cardNumber / 13) == (int)deckSuit::SPADE){
                spadeCardsCount += 1;
                spadeCardValue = cardNumber;
            }
        }
        if(spadeCardsCount == 1){
            turnAlreadyDetermined.emplace_back(player.id, spadeCardValue);
            player.cards.remove(spadeCardValue);
            flushedCards.emplace_back(spadeCardsCount);
            continue;
        }
        if(spadeCardsCount == 0){
            player.turnExpected = true;
            player.turnTypeExpected = turnType::FIRSTROUNDANY;
        }else{
            player.turnExpected = true;
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
        int handSize = p.cards.size();
        //STEP 2;
        out.write(reinterpret_cast<char*>(&handSize),sizeof(handSize));
        for(auto card: p.cards){
            //STEP 3;
            out.write(reinterpret_cast<char*>(&card), sizeof(card));
        }
        for(auto& turnSequenceId: gamePlayersData){
            int sequenceId = turnSequenceId.id;
            //STEP 4; //Turn Sequence
            out.write(reinterpret_cast<char*>(&sequenceId), sizeof(sequenceId));
        }
        int turnAlreadyDeterminedSize = turnAlreadyDetermined.size();
        //STEP 5;
        out.write(reinterpret_cast<char*>(&turnAlreadyDeterminedSize), sizeof(turnAlreadyDeterminedSize));
        for(auto& alreadyDetermined: turnAlreadyDetermined){
            int id = std::get<0>(alreadyDetermined);
            int cardNumber = std::get<1>(alreadyDetermined);
            //STEP 6;
            out.write(reinterpret_cast<char*>(&id), sizeof(id));
            //STEP 7;
            out.write(reinterpret_cast<char*>(&cardNumber), sizeof(cardNumber));
        }

        for(auto& gp: gamePlayersData){
            int gpId = gp.id;
            //STEP 8;
            out.write(reinterpret_cast<char*>(&gpId), sizeof(gpId));
            int gpCardsSize = gp.cards.size();
            //STEP 9;
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

void serverLobbyManager::managementGAMETURNCLIENTReceived(int receivedCardNumber, int sessionId) {
    //iterating over gamePlayerData
    auto turnReceivedPlayer = std::find_if(gamePlayersData.begin(), gamePlayersData.end(),
                                               [sessionId](playerData& p){
        return p.id == sessionId;
    });//Be Careful Of turnReceivedPlayer Invalidation during the function
    assert(turnReceivedPlayer != gamePlayersData.end() && "no gamePlayerData id matches with sessionId");
    auto iter = std::find(turnReceivedPlayer->cards.begin(), turnReceivedPlayer->cards.end(), receivedCardNumber);
    if(iter == turnReceivedPlayer->cards.end()) {
        std::cout << "A Client Turn Received But Not Acted Upon. Card Number not in card list user.\r\n";
        return;
    }
    if(firstRound){
        if(turnReceivedPlayer->turnTypeExpected == turnType::FIRSTROUNDANY){
            doTurnReceivedOfFirstRound(turnReceivedPlayer, receivedCardNumber);//paste that code after this line
        }else{
            if(receivedCardNumber/13 != (int)deckSuit::SPADE) {
                std::cout << "A Turn Received But Not Acted Upon. In First Turn Client"
                             "Turned Other Card When It Had The Spade\r\n";
                return;
            }
            doTurnReceivedOfFirstRound(turnReceivedPlayer, receivedCardNumber);//paste that code after this line
        }

    }else{
        Turn(turnReceivedPlayer, receivedCardNumber);
    }
}

void
serverLobbyManager::doTurnReceivedOfFirstRound(
        std::vector<playerData>::iterator turnReceivedPlayer, int receivedCardNumber) {
    turnCardNumberOfGamePlayerIterator(turnReceivedPlayer, receivedCardNumber);
    roundTurns.emplace_back(receivedCardNumber, turnReceivedPlayer->id);
    if(roundTurns.size() != gamePlayersData.size()) {
        return;
    }
    roundTurns.clear();
    firstRound = false;
    for(auto t: roundTurns){
        if(std::get<0>(t)/13 == 0){
            //This is ace of spade
            for(auto it = gamePlayersData.begin(); it!=gamePlayersData.end(); ++it){
                if(it->id == std::get<1>(t)){
                    newRoundTurn(it);
                    break;
                }
            }
            break;
        }
    }
}

void serverLobbyManager::newRoundTurn(std::vector<playerData>::iterator currentGamePlayer){
    if(currentGamePlayer->cards.size() == 1){
        currentGamePlayer->turnTypeExpected = turnType::ROUNDFIRSTTURN;
        suitOfTheRound = static_cast<deckSuit>(*currentGamePlayer->cards.begin());
        Turn(currentGamePlayer, *currentGamePlayer->cards.begin());
    }else{
        currentGamePlayer->turnTypeExpected = turnType::ROUNDFIRSTTURN;
        currentGamePlayer->turnExpected = true;
    }
}

void serverLobbyManager::Turn(std::vector<playerData>::iterator currentTurnPlayer, int receivedCardNumber){
    if(currentTurnPlayer->turnTypeExpected == turnType::ROUNDFIRSTTURN){ //First Turn Of Round Was Expected
        performFirstOrMiddleTurn(currentTurnPlayer, receivedCardNumber, true);
    }else if (currentTurnPlayer->turnTypeExpected == turnType::ROUNDMIDDLETURN){
        performFirstOrMiddleTurn(currentTurnPlayer, receivedCardNumber, false);
    }else if(currentTurnPlayer->turnTypeExpected == turnType::ROUNDLASTTURN){
        performLastOrThullaTurn(currentTurnPlayer,receivedCardNumber,true);
    }else if(currentTurnPlayer->turnTypeExpected == turnType::THULLA){//A Thulla Was Expected
        performLastOrThullaTurn(currentTurnPlayer,receivedCardNumber,true);
    }
}

void serverLobbyManager::performFirstOrMiddleTurn(
        std::vector<playerData>::iterator currentTurnPlayer, int receivedCardNumber, bool firstTurn){


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
        suitOfTheRound = static_cast<deckSuit>(receivedCardNumber/13);
    }else{
        assert(!roundTurns.empty() && "If it is not first turn then roundTurns should not be empty");
        if(receivedCardNumber/13 != (int) suitOfTheRound){
            std::cout<<"A Turn Received But Not Acted Upon Because Card is not of the required suit"<<std::endl;
        }
    }

    turnCardNumberOfGamePlayerIterator(currentTurnPlayer, receivedCardNumber);
    roundTurns.emplace_back(receivedCardNumber, currentTurnPlayer->id);


    int suitCardsCount = 0;
    int mayGetUsedCard = -1;
    for(auto& cardNumber: nextGamePlayerIterator->cards) {
        if (cardNumber / 13 == (int) suitOfTheRound) {
            suitCardsCount += 1;
            mayGetUsedCard = cardNumber;
        }
    }

    if(suitCardsCount == 0) {
        nextGamePlayerIterator->turnTypeExpected = turnType::THULLA;
        if (nextGamePlayerIterator->cards.size() == 1) {
            Turn(nextGamePlayerIterator, mayGetUsedCard);
        } else {
            currentTurnPlayer->turnExpected = true;
        }
    }else if(suitCardsCount == 1){
        if(roundTurns.size() == gamePlayersData.size() -1){
            nextGamePlayerIterator->turnTypeExpected = turnType::ROUNDLASTTURN;
        }else{
            nextGamePlayerIterator->turnTypeExpected = turnType::ROUNDMIDDLETURN;
        }
        Turn(nextGamePlayerIterator, mayGetUsedCard);
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
        std::vector<playerData>::iterator currentTurnPlayer, int receivedCardNumber, bool lastTurn){
    std::vector<playerData>::iterator roundKing;
    if(lastTurn){
        if(receivedCardNumber/13 != (int) suitOfTheRound){
            std::cout<<"A Turn Received But Not Acted Upon Because Card is not of the required suit"<<std::endl;
            return;
        }
        roundKing = roundKingGamePlayerDataIterator();
        for(auto cn: roundTurns){
            flushedCards.emplace_back(std::get<0>(cn));
        }
        flushedCards.emplace_back(receivedCardNumber);
    }else{
        roundKing = roundKingGamePlayerDataIterator();
        for(auto cn: roundTurns){
            roundKing->cards.emplace_back(std::get<0>(cn));
        }
        roundKing->cards.emplace_back(receivedCardNumber);
    }
    turnCardNumberOfGamePlayerIterator(currentTurnPlayer, receivedCardNumber);

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
                                                            int receivedCardNumber){
    //send turn to every-one except one
    //remove the card from turnReceiverPlayer.cards
    //set TurnExpected of current = false;
    //emplace back in roundTurns

    sendGAMETURNSERVERTOAllExceptOne(receivedCardNumber, turnReceivedPlayer->id);
    turnReceivedPlayer->cards.remove(receivedCardNumber);
    turnReceivedPlayer->turnExpected = false;
}

std::vector<playerData>::iterator serverLobbyManager::roundKingGamePlayerDataIterator(){
    int highestCardNumber = std::get<0>(*roundTurns.begin());
    int highestCardHolderId = std::get<1>(*roundTurns.begin());
    for(auto turns: roundTurns){
        int cardNumber = std::get<0>(turns) % 13;
        if(cardNumber == 0 ){
            highestCardHolderId = std::get<1>(turns);
            break;
        }else{
            if(cardNumber > highestCardNumber){
                highestCardNumber = cardNumber;
                highestCardHolderId = std::get<1>(turns);
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