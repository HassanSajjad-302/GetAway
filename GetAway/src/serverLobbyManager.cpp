
#include <serverLobbyManager.hpp>
#include <utility>

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
    auto tup = std::tuple(playerNameAdvanced, lobbySession);
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
    //Tell EveryOne SomeOne has Joined In
    sendSelfAndStateToOneAndUpdateToRemaining();
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
    //STEP 2; //MESSAGE RECEIVED AND FORWARDED TO OTHER PLAYERS
    char arr[manager.receivedPacketSize - 4];
    in.getline(arr, manager.receivedPacketSize -4);
    manager.chatMessageReceived = std::string(arr);
    manager.sendChatMessageToAllExceptSenderItself();
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
            //STEP 5;
            for(auto& gamePlayer : manager.gameData){
                //STEP 6;
                int id = gamePlayer.first;
                out.write(reinterpret_cast<char *>(&id), sizeof(id));
                //STEP 7;
                out << std::get<0>(gamePlayer.second) << std::endl;
            }
            break;
        }
        case lobbyMessageType::UPDATE:{
            //STEP 1;
            lobbyMessageType t = lobbyMessageType::UPDATE;
            out.write(reinterpret_cast<char*>(&t), sizeof(t));
            //STEP 2;
            auto map_ptr = manager.gameData.find(manager.excitedSessionId);
            int id = map_ptr->first;
            out.write(reinterpret_cast<char*>(&id), sizeof(id));
            //STEP 3;
            out << std::get<0>(map_ptr->second) << std::endl;
            break;
        }
        case lobbyMessageType::CHATMESSAGE: {
            //STEP 1;
            lobbyMessageType t = lobbyMessageType::CHATMESSAGE;
            out.write(reinterpret_cast<char*>(&t), sizeof(t));
            //STEP 2;
            out << manager.chatMessageReceived << std::endl;
            break;
        }
    }
    return out;
}

//excitedSessionId is the one to send state to and update of it to remaining
void serverLobbyManager::sendSelfAndStateToOneAndUpdateToRemaining(){
    std::cout <<"New Player Joined " << playerNameFinal <<std::endl;
    messageSendingType = lobbyMessageType::SELFANDSTATE;
    std::get<1>(gameData.find(excitedSessionId)->second)->sendMessage(&serverLobbyManager::uselessWriteFunction);
    messageSendingType = lobbyMessageType::UPDATE;
    for(auto& player: gameData){
        if(player.first != excitedSessionId){
            std::get<1>(player.second)->sendMessage(&serverLobbyManager::uselessWriteFunction);
        }
    }
}

void serverLobbyManager::sendChatMessageToAllExceptSenderItself(){
    messageSendingType = lobbyMessageType::CHATMESSAGE;
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
