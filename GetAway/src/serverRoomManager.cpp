
#include <serverPF.hpp>
#include <sati.hpp>
#include <serverHome.hpp>
#include "serverRoomManager.hpp"

serverRoomManager::serverRoomManager(std::shared_ptr <serverListener> serverlistener_, asio::io_context &io_):
        serverlistener(std::move(serverlistener_)), io{io_} {
    chatManager = std::make_shared<serverChatManager>(players);
}

void serverRoomManager::shutDown() {
    constants::Log("UseCount of serverlistener from serverLobbyManager {}", serverlistener.use_count());
    serverlistener.reset();
    for(auto &p: players){
        std::get<1>(p.second)->sock.shutdown(asio::socket_base::shutdown_both);
        std::get<1>(p.second)->sock.close();
        constants::Log("UseCount of serverLobbySession from serverLobbyManager {}", std::get<1>(p.second).use_count());
        std::get<1>(p.second).reset();
    }
}

void serverRoomManager::setPlayerNameAdvanced(std::string playerNameAdvacned_) {
    playerNameAdvanced = std::move(playerNameAdvacned_);
}

int
serverRoomManager::
join(std::shared_ptr<session<serverRoomManager, true>> roomSession)
{
    roomSession->receiveMessage();
    std::tuple<std::string, std::shared_ptr<session<serverRoomManager, true>>> tup(playerNameAdvanced, std::move(roomSession));
    int id;
    std::string playerNameFinal;
    if(players.empty())
    {
        id = 0;
        players.emplace(id,tup);
        playerNameFinal = std::move(playerNameAdvanced);
    }
    else{
        id = players.rbegin()->first + 1;
        auto pair = players.emplace(id,tup);
        playerNameFinal = std::move(playerNameAdvanced);
        //TODO
        //Currently, There is no policy for handling players with different Names.
        //Because of having same Id, I currently don't care. Though this is the only
        //reason server resends the playerName.
    }
    //Tell EveryOne SomeOne has Joined In
    managementJoin(id, playerNameFinal);

    if(players.size() == 2){
        sati::getInstance()->setBase(this, appState::LOBBY);
        serverPF::setLobbyMainTwoOrMorePlayers();
        setInputType(inputType::SERVERLOBBYTWOORMOREPLAYERS);
    }
    return id;
}

void
serverRoomManager::
leave(int id)
{
    if(!gameStarted){
        sendPLAYERLEFTToAllExceptOne(id);
        resourceStrings::print("Player Left: " + std::get<0>(players.find(id)->second) + "\r\n");
        players.erase(players.find(id));
        if(players.size() == 1){
            goBackToServerListener();
        }
    }else{
        resourceStrings::print("Player Left During The Game\r\n");
        //todo
        //handle player left during the game by sending a player left message.
        exit(-1);
    }
}

void serverRoomManager::packetReceivedFromNetwork(std::istream &in, int receivedPacketSize, int sessionId){
    mtc messageTypeReceived;
    //STEP 1;
    in.read(reinterpret_cast<char*>(&messageTypeReceived), sizeof(messageType));
    if(gameStarted && messageTypeReceived == mtc::GAME){
        lobbyManager->packetReceivedFromNetwork(in, receivedPacketSize, sessionId);
    }else if(messageTypeReceived == mtc::MESSAGE){
        lobbyManager->packetReceivedFromNetwork(in, receivedPacketSize, sessionId);
    }
    else{
        resourceStrings::print("Unexpected Packet Type Received in class serverRoomManager\r\n");
    }
}

//excitedSessionId is the one to send state to and update of it to remaining
void serverRoomManager::managementJoin(int excitedSessionId, const std::string& playerNameFinal) {

    auto playerSession = std::get<1>(players.find(excitedSessionId)->second);
    std::ostream& out = playerSession->out;

    //STEP 1;
    messageType t = messageType::SELFANDSTATE;
    out.write(reinterpret_cast<char*>(&t), sizeof(t));
    //STEP 2;
    out.write(reinterpret_cast<char *>(&excitedSessionId), sizeof(excitedSessionId));
    //STEP 3;
    out << playerNameFinal << std::endl;
    //STEP 4;
    int size = players.size();
    out.write(reinterpret_cast<char *>(&size), sizeof(size));
    for(auto& gamePlayer : players){
        //STEP 5;
        int id = gamePlayer.first;
        out.write(reinterpret_cast<char *>(&id), sizeof(id));
        //STEP 6;
        out << std::get<0>(gamePlayer.second) << std::endl;
    }
    playerSession->sendMessage(&serverRoomManager::uselessWriteFunction);

    sendPLAYERJOINEDToAllExceptOne(excitedSessionId);
    resourceStrings::print("Player Joined: " + std::get<0>(players.find(excitedSessionId)->second) + "\r\n");
}


void serverRoomManager::sendPLAYERJOINEDToAllExceptOne(int excitedSessionId) {

    for(auto& player: players){
        if(player.first != excitedSessionId){
            auto playerSession = std::get<1>(player.second);
            std::ostream& out = playerSession->out;

            //STEP 1;
            messageType t = messageType::PLAYERJOINED;
            out.write(reinterpret_cast<char*>(&t), sizeof(t));
            //STEP 2;
            auto mapPtr = players.find(excitedSessionId);
            int id = mapPtr->first;
            out.write(reinterpret_cast<char*>(&id), sizeof(id));
            //STEP 3;
            out << std::get<0>(mapPtr->second) << std::endl;

            playerSession->sendMessage(&serverRoomManager::uselessWriteFunction);
        }
    }
}

void serverRoomManager::sendPLAYERLEFTToAllExceptOne(int excitedSessionId) {
    for(auto& player: players){
        if(player.first != excitedSessionId){
            auto playerSession = std::get<1>(player.second);
            std::ostream& out = playerSession->out;

            messageType t = messageType::PLAYERLEFT;
            //STEP 1;
            out.write(reinterpret_cast<char*>(&t), sizeof(t));
            auto mapPtr = players.find(excitedSessionId);
            int id = mapPtr->first;
            //STEP 2;
            out.write(reinterpret_cast<char*>(&id), sizeof(id));

            playerSession->sendMessage(&serverRoomManager::uselessWriteFunction);
        }
    }
}

void serverRoomManager::uselessWriteFunction(int id) {

}

void serverRoomManager::goBackToServerListener(){
    serverlistener->registerForInputReceival();
}

void serverRoomManager::setInputType(inputType type){
    sati::getInstance()->setInputType(type);
    inputTypeExpected = type;
}

void serverRoomManager::input(std::string inputString, inputType inputReceivedType){
    if(inputReceivedType == inputTypeExpected){
        if(inputReceivedType == inputType::SERVERLOBBYTWOORMOREPLAYERS){
            int input;
            if(constants::inputHelper(inputString, 1, 3,inputType::SERVERLOBBYTWOORMOREPLAYERS,
                                      inputType::SERVERLOBBYTWOORMOREPLAYERS, input)){
                if(input == 1){
                    //Start The Game
                    serverlistener->shutdownAcceptorAndProbe();
                    lobbyManager = std::make_shared<serverLobbyManager>(players, *this);
                    setInputType(inputType::GAMEINT);
                    sati::getInstance()->setBase(this, appState::GAME);
                    serverPF::setGameMain();
                }else if(input == 2){
                    //Close Server
                    serverlistener->shutdown();
                    std::make_shared<serverHome>(serverHome(io))->run();
                }else{
                    //Exit
                    serverlistener->shutdown();
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
                    serverlistener->shutdown();
                }else{
                    serverlistener->shutdown();
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