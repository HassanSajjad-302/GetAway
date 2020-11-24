
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
    if(gameData.size() == 2){
        std::this_thread::sleep_for(std::chrono::seconds(3));
        net::post(std::get<1>(gameData.begin()->second)->sock.get_executor(), [self = this](){self->startGame();});
        /*gameTimer.async_wait([self= this](errorCode ec){
            self->startGame();
        });*/
    }
    return id;
}

void
serverLobbyManager::
leave(int id)
{
    excitedSessionId = id;
    printPlayerLeft();
    sendPlayerLeftToAllExceptOne();
    gameData.erase(gameData.find(id));
}

std::istream &operator>>(std::istream &in, serverLobbyManager &manager) {
    //STEP 1;
    lobbyMessageType messageTypeReceived;
    in.read(reinterpret_cast<char*>(&messageTypeReceived), sizeof(lobbyMessageType));
    if(messageTypeReceived == lobbyMessageType::CHATMESSAGE){
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
    }
    else{
        std::cout<<"Unexpected Packet Type Received in class clientLobbyManager"<<std::endl;
    }

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
        case lobbyMessageType::GAMEFIRSTTURNSERVER: {
            lobbyMessageType t = lobbyMessageType::GAMEFIRSTTURNSERVER;
            //STEP 1;
            out.write(reinterpret_cast<char*>(&t), sizeof(t));
            auto &p = manager.gamePlayersData[manager.currentIndexGamePlayersData];
            int handSize = p.cards.size();
            spdlog::info("Sending Messages to Player {}", std::get<0>(manager.gameData.find(manager.excitedSessionId)->second));
            spdlog::info("Hand Size is {d}", handSize);
            //STEP 2;
            out.write(reinterpret_cast<char*>(&handSize),sizeof(handSize));
            spdlog::info("Cards Written Are Following");
            for(auto card: p.cards){
                //STEP 3;
                spdlog::info("Card Number Is {}", card);
                out.write(reinterpret_cast<char*>(&card), sizeof(card));
            }
            for(auto& turnSequenceId: manager.gamePlayersData){
                int sequenceId = turnSequenceId.id;
                spdlog::info("Turn Sequence {d}", turnSequenceId.id);
                //STEP 4; //Turn Sequence
                out.write(reinterpret_cast<char*>(&sequenceId), sizeof(sequenceId));
            }
            std::vector<std::tuple<int, int>>turnAlreadyDetermined; //id and cardValue
            for(auto& playerData: manager.gamePlayersData){
                if(playerData.playerTurnType == turnType::TURNNOOTHERPOSSIBLE){
                    turnAlreadyDetermined.emplace_back(playerData.id, playerData.cardValueAuto);
                }
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

//Game Functions
void serverLobbyManager::startGame(){
    initializeGame();
    //At this point playerGameCards is ready to be distributed.
    errorCode ec;
    serverlistener->sock.shutdown(tcp::socket::shutdown_both, ec);
    serverlistener->sock.close();
    checkForAutoFirstTurn();
    doFirstTurn();
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

void serverLobbyManager::checkForAutoFirstTurn(){
    for(auto& player: gamePlayersData){
        int spadeCardsCount = 0;
        int spadeCardValue;
        for(auto& cardNumber: player.cards){
            if(cardNumber == 26){ // 26 is calculated by cardNumber/13 == suitValues::spade
                player.playerTurnType = turnType::TURNNOOTHERPOSSIBLE;
                player.cardValueAuto = cardNumber;
                break;
            }
            if((cardNumber / 13) == (int)deckSuit::SPADE){
                spadeCardsCount += 1;
                spadeCardValue = cardNumber;
            }
        }
        if(spadeCardsCount == 1){
            player.playerTurnType = turnType::TURNNOOTHERPOSSIBLE;
            player.cardValueAuto =  spadeCardValue;
        }
    }
}

void serverLobbyManager::doFirstTurn(){
    messageSendingType = lobbyMessageType::GAMEFIRSTTURNSERVER;
    for(int i=0; i<gamePlayersData.size(); ++i){
        auto& player = gamePlayersData[i];
        currentIndexGamePlayersData = i;
        excitedSessionId = gamePlayersData[i].id;
        std::get<1>(gameData.find(excitedSessionId)->second)->sendMessage(&serverLobbyManager::uselessWriteFunction);
        if(player.playerTurnType == turnType::TURNNOOTHERPOSSIBLE){
#ifndef NDEBUG
            int priorSize = player.cards.size();
#endif
            int p = player.cards.remove(player.cardValueAuto);
#ifndef NDEBUF
            int afterSize = player.cards.size();
            assert(priorSize>afterSize && "No Element Was Removed Error");
#endif
            flushedCards.emplace_back(player.cardValueAuto);

        }else{ //In first turn turnType::TURNPLAYEROFFLINE not possible, so it is turnType::TURNBYPLAYER
            player.messageTypeExpectedGame.push_back(lobbyMessageType::GAMETURNCLIENT);
            std::get<1>(gameData.find(excitedSessionId)->second)->receiveMessage();
        }
    }
    //There will be some method which will decide whether receive has completed which will be passed as receive handler.
    //In that handler we will set the firstTurn bool which will tell us that firstTurn has completed for playerLeftManagement
}

void serverLobbyManager::handlerPlayerLeft(int id){
    if(firstTurn){
        //
    }
}

void serverLobbyManager::checkForNextTurn(int nextPlayerId){

}
