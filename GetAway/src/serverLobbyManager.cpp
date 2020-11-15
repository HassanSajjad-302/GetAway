
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
#ifdef LOG
    spdlog::info("{}\t{}\t{}",__FILE__,__FUNCTION__ ,__LINE__);
#endif
    lobbySession->receiveMessage();
    auto tup = std::tuple(playerNameAdvanced, std::move(lobbySession));
    int id;
    if(gameData.empty())
    {
        id = 0;
        excitedSessionId = id;
        gameData.emplace(id,tup);
        playerNameFinal = std::move(playerNameAdvanced);
    }
    else{
        id = gameData.rbegin()->first + 1;
        excitedSessionId = id;
        auto pair = gameData.emplace(id,tup);
        playerNameFinal = std::move(playerNameAdvanced);
        //TODO
        //Currently, There is no policy for handling players with different Names.
        //Because of having same Id, I currently don't care. Though this is the only
        //reason server resends the playerName.
    }
    printPlayerJoined();
    //Tell EveryOne SomeOne has Joined In
    sendSelfAndStateToOneAndPlayerJoinedToRemaining();
    numOfPlayers +=1;
    if(numOfPlayers == 2){
        timer.async_wait([self= this](errorCode ec){
            self->startGame();
        });
    }
#ifdef LOG
    spdlog::info("{}\t{}\t{}",__FILE__,__FUNCTION__ ,__LINE__);
#endif
    return id;
}

void
serverLobbyManager::
leave(int id)
{
#ifdef LOG
    spdlog::info("{}\t{}\t{}",__FILE__,__FUNCTION__ ,__LINE__);
#endif

    excitedSessionId = id;
    printPlayerLeft();
    sendPlayerLeftToAllExceptOne();
    gameData.erase(gameData.find(id));

#ifdef LOG
    spdlog::info("{}\t{}\t{}",__FILE__,__FUNCTION__ ,__LINE__);
#endif
}

std::istream &operator>>(std::istream &in, serverLobbyManager &manager) {
    //STEP 1;
    lobbyMessageType messageTypeReceived;
    in.read(reinterpret_cast<char*>(&messageTypeReceived), sizeof(lobbyMessageType));
    if(messageTypeReceived != lobbyMessageType::CHATMESSAGE){
        std::cout<<"Unexpected Packet Type Received in class clientLobbyManager"<<std::endl;
    }
    //STEP 2;
    int senderId;
    in.read(reinterpret_cast<char*>(&senderId), sizeof(senderId));
    assert(senderId == manager.excitedSessionId);
    //STEP 3;
    int arrSize = 4 + 4 - 1; //4 for messageType, 4 for Id, 1 for getline
    char arr[manager.receivedPacketSize - 4];
    in.getline(arr, manager.receivedPacketSize -4);
    manager.chatMessageReceived = std::string(arr);
    manager.sendChatMessageToAllExceptSenderItself();
    std::cout<<"Message Sent To All Clients " << std::endl;
    std::get<1>(manager.gameData.find(manager.excitedSessionId)->second)->receiveMessage();
    return in;
}

std::ostream &operator<<(std::ostream &out, serverLobbyManager &manager) {
    switch(manager.messageSendingType){
        case lobbyMessageType::SELFANDSTATE: {
            //STEP 1;
            lobbyMessageType t = lobbyMessageType::SELFANDSTATE;
            out.write(reinterpret_cast<char*>(&t), sizeof(t));
            //STEP 2;
            out.write(reinterpret_cast<char *>(&manager.excitedSessionId), sizeof(manager.excitedSessionId));
            //STEP 3;
            out << manager.playerNameFinal << std::endl;
            //STEP 4;
            int size = manager.gameData.size();
            out.write(reinterpret_cast<char *>(&size), sizeof(size));
            for(auto& gamePlayer : manager.gameData){
                //STEP 5;
                int id = gamePlayer.first;
                out.write(reinterpret_cast<char *>(&id), sizeof(id));
                //STEP 6;
                out << std::get<0>(gamePlayer.second) << std::endl;
            }
            break;
        }
        case lobbyMessageType::PLAYERJOINED:{
            //STEP 1;
            lobbyMessageType t = lobbyMessageType::PLAYERJOINED;
            out.write(reinterpret_cast<char*>(&t), sizeof(t));
            //STEP 2;
            auto mapPtr = manager.gameData.find(manager.excitedSessionId);
            int id = mapPtr->first;
            out.write(reinterpret_cast<char*>(&id), sizeof(id));
            //STEP 3;
            out << std::get<0>(mapPtr->second) << std::endl;
            break;
        }
        case lobbyMessageType::PLAYERLEFT:{
            lobbyMessageType t = lobbyMessageType::PLAYERLEFT;
            //STEP 1;
            out.write(reinterpret_cast<char*>(&t), sizeof(t));
            auto mapPtr = manager.gameData.find(manager.excitedSessionId);
            int id = mapPtr->first;
            //STEP 2;
            out.write(reinterpret_cast<char*>(&id), sizeof(id));
            break;
        }
        case lobbyMessageType::CHATMESSAGEID: {
            //STEP 1;
            lobbyMessageType t = lobbyMessageType::CHATMESSAGEID;
            out.write(reinterpret_cast<char*>(&t), sizeof(t));
            //STEP 2;
            out.write(reinterpret_cast<char *>(&manager.excitedSessionId), sizeof(manager.excitedSessionId));
            //STEP 3;
            out << manager.chatMessageReceived << std::endl;
            break;
        }
        case lobbyMessageType::FIRSTGAMEMESSAGE: {
            lobbyMessageType t = lobbyMessageType::FIRSTGAMEMESSAGE;
            //STEP 1;
            out.write(reinterpret_cast<char*>(&t), sizeof(t));
            auto &p = manager.playerGameCards.find(manager.excitedSessionId)->second;
            int handSize = p.getCardCount();
            //STEP 2;
            out.write(reinterpret_cast<char*>(&handSize),sizeof(handSize));
            for(auto card: p.getCards()){
                //STEP 3;
                out.write(reinterpret_cast<char*>(&card), sizeof(card));
            }
        }
    }
    return out;
}

//excitedSessionId is the one to send state to and update of it to remaining
void serverLobbyManager::sendSelfAndStateToOneAndPlayerJoinedToRemaining(){
    messageSendingType = lobbyMessageType::SELFANDSTATE;
    std::get<1>(gameData.find(excitedSessionId)->second)->sendMessage(&serverLobbyManager::uselessWriteFunction);
    messageSendingType = lobbyMessageType::PLAYERJOINED;
    for(auto& player: gameData){
        if(player.first != excitedSessionId){
            std::get<1>(player.second)->sendMessage(&serverLobbyManager::uselessWriteFunction);
        }
    }
}

void serverLobbyManager::sendChatMessageToAllExceptSenderItself(){
    messageSendingType = lobbyMessageType::CHATMESSAGEID;
    for(auto& player: gameData){
        if(player.first != excitedSessionId){
            std::get<1>(player.second)->sendMessage(&serverLobbyManager::uselessWriteFunction);
        }
    }
}

void serverLobbyManager::sendPlayerLeftToAllExceptOne(){
    messageSendingType = lobbyMessageType::PLAYERLEFT;
    for(auto& player: gameData){
        if(player.first != excitedSessionId){
            std::get<1>(player.second)->sendMessage(&serverLobbyManager::uselessWriteFunction);
        }
    }
}

void serverLobbyManager::uselessWriteFunction(int id){

}
void serverLobbyManager::setPlayerNameAdvanced(std::string advancedPlayerName_) {
    playerNameAdvanced = std::move(advancedPlayerName_);
}



//PRINTING FUNCTIONS


void serverLobbyManager::printPlayerJoined(){
    std::cout << "Player Joined: " << std::get<0>(gameData.find(excitedSessionId)->second) << std::endl;
}

void serverLobbyManager::printPlayerLeft(){
    std::cout << "Player Left: " << std::get<0>(gameData.find(excitedSessionId)->second) << std::endl;
}

void serverLobbyManager::initializeGame(){
    auto rd = std::random_device {};
    auto rng = std::default_random_engine { rd() };

    int numberOfPlayersWithExtraCards = 52 % numOfPlayers;
    int numberOfPlayersWithNormalNumberOfCards = 52/numOfPlayers;

    std::vector<int> playersIdList;
    for(const auto& player: gameData){
        playersIdList.push_back(player.first);
    }
    std::shuffle(playersIdList.begin(), playersIdList.end(), rng);
    turnIfo.setTurnOrder(playersIdList);

    std::vector<int> playerWithExtraCardIdsList;
    for(int i=0;i<numberOfPlayersWithExtraCards;++i){
        playerWithExtraCardIdsList.push_back(playersIdList[i]);
    }

    int cards[52];
    for(int i=0;i<52;++i){
        cards[i] = i;
    }
    std::shuffle(cards, cards + 52, rng);

    int normalNumberOfCards = 52/numOfPlayers;

    int cardsCount = 0;
    for(auto player: gameData){
        auto p = playerCards();
        if(std::find(playerWithExtraCardIdsList.begin(),playerWithExtraCardIdsList.end(),player.first) !=
           playerWithExtraCardIdsList.end()){
            for(int i=cardsCount; i<normalNumberOfCards+1; ++i){
                p.addCard(cards[i]);
            }
        }else{
            for(int i=cardsCount; i<normalNumberOfCards; ++i){
                p.addCard(cards[i]);
            }
        }
        playerGameCards.emplace(player.first,std::move(p));
    }

#ifndef NDEBUG
    int size = 0;
    for(auto& p: playerGameCards){
        size += p.second.getCardCount();
    }
    assert(size == 52 && "Sum Of Cards distributed not equal to 52");
#endif

}


//Game Functions
void serverLobbyManager::startGame(){
    initializeGame();
    //At this point playerGameCards is ready to be distributed.
    serverlistener->sock.cancel();
    checkForAutoFirstTurn();
    doFirstTurn();
}

//We need to check for in first turn whose players turn can be performed by the computer. This will be checked by
//checking the integer value of 26 in the map of playerGameCards.
//We will then make and send another vector<int> which will have the list of those ids whose turn is also determined.

void serverLobbyManager::checkForAutoFirstTurn(){
    for(auto& player: playerGameCards){
        int spadeCardsCount = 0;
        int spadeCardValue;
        for(auto& cardNumber: player.second.getCards()){
            if(cardNumber == 26){ // 26 is calculated by cardNumber/13 == suitValues::spade
                turnIfo.setCurrentPlayer(player.first);
                break;
            }
            if((cardNumber / 13) == (int)deckSuit::SPADE){
                spadeCardsCount += 1;
                spadeCardValue = cardNumber;
            }
            if(spadeCardsCount == 1){
                turnAlreadyDeterminedIdsInFirstTurn.emplace_back(cardNumber, spadeCardValue);
            }
        }
    }
}

void serverLobbyManager::doFirstTurn(){
    lobbyMessageType t = lobbyMessageType::FIRSTGAMEMESSAGE;
    for(auto& player: gameData){
        excitedSessionId = player.first;
        std::get<1>(player.second)->sendMessage(&serverLobbyManager::uselessWriteFunction);
    }
    //Flush Some
    //Receive
}

void serverLobbyManager::checkForNextTurn(int nextPlayerId){

}

void turnMeta::setTurnOrder(std::vector<int> Ids) {
    for(int & id : Ids){
        turnOrder.emplace_back(id);
    }
    currentPlayer = 0;
}


void turnMeta::setCurrentPlayer(int playerId){
    for(int i=0; i<turnOrder.size(); ++i){
        if(turnOrder[i] == playerId){
            currentPlayer = playerId;
        }
    }
}
int turnMeta::nextPlayerId() {
    if(currentPlayer == turnOrder.size() -1){
        currentPlayer = 0;
    }
    else{
        currentPlayer += 1;
    }
    return turnOrder[currentPlayer];
}

int turnMeta::getCurrentPlayerId() {
    return currentPlayer;
}
