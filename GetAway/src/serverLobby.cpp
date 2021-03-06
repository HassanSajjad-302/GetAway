
#include <sati.hpp>
#include <home.hpp>
#include "serverLobby.hpp"
#include "serverListener.hpp"
#include "serverChat.hpp"

serverLobby::serverLobby(serverListener& serverlistener_, asio::io_context &io_, bool serverOnly_,
                         constants::gamesEnum gameSelected_):serverlistener(serverlistener_), io{io_},
                         gameSelected(gameSelected_), serverOnly(serverOnly_) {
    chatManagerPtr = std::make_unique<serverChat>(players);
}

void serverLobby::shutDown() {
    for(auto &p: players){
        std::get<1>(p.second)->sock.shutdown(asio::socket_base::shutdown_both);
        std::get<1>(p.second)->sock.close();
        std::get<1>(p.second).reset();
    }
}

void serverLobby::newConnectionReceived(asio::ip::tcp::socket sock) {
    auto lobbySession = std::make_unique<serverSession<serverLobby>>(std::move(sock), *this, maxID);
    lobbySession->receiveMessage();
    yetToBePromotedSession.emplace(maxID, std::move(lobbySession));
    ++maxID;
}

void
serverLobby::
leave(int id)
{
    if(!gameStarted){
        auto s = yetToBePromotedSession.find(id);
        if(s == yetToBePromotedSession.end()){
            players.erase(players.find(id));
            sendPLAYERLEFTToAllExceptOne(id);
            resourceStrings::print("Player Left: " + std::get<0>(players.find(id)->second) + "\r\n");
            if(serverOnly){
                if(players.size() == 1){
                    serverlistener.registerForInputReceival();
                }
            }
        }else{
            yetToBePromotedSession.erase(s);
        }
    }else{
        players.erase(players.find(id));
        resourceStrings::print("Player Left During The Game\r\n");
        sendPLAYERLEFTDURINGGAMEToAllExceptOne(id);
        gameExitFinished();
    }
}

void serverLobby::packetReceivedFromNetwork(std::istream &in, int receivedPacketSize, int sessionId){
    mtc messageTypeReceived;
    //STEP 1;
    in.read(reinterpret_cast<char*>(&messageTypeReceived), sizeof(messageType));
    if(messageTypeReceived == mtc::LOBBY){
        //STEP 2;
        //TODO
        char arr[61]; //This constant will be fed from somewhere else but one is added.
        in.getline(arr,61);
        joinedPlayerName = std::string(arr);
        //todo
        //currently no policy for handling players with same names.
        auto s = std::move(yetToBePromotedSession.find(sessionId)->second);
        //s->receiveMessage();
        yetToBePromotedSession.erase(sessionId);
        players.emplace(sessionId, std::make_tuple(std::move(joinedPlayerName), std::move(s)));
        managementJoin(sessionId, std::get<0>(players.find(sessionId)->second));

        if(players.size() == 2){
            if(serverOnly){
                sati::getInstance()->setBase(this, appState::LOBBY);
                PF::setLobbyMainTwoOrMorePlayers();
                setInputType(inputType::SERVERLOBBYTWOORMOREPLAYERS);
            }
        }
    }
    else if(messageTypeReceived == mtc::GAME){
        if(gameSelected == constants::gamesEnum::GETAWAY){
            serverGetAwayPtr->packetReceivedFromNetwork(in, receivedPacketSize, sessionId);
        }else if(gameSelected == constants::gamesEnum::BLUFF){
            serverBluffPtr->packetReceivedFromNetwork(in, receivedPacketSize, sessionId);
        }
    }else if(messageTypeReceived == mtc::MESSAGE){
        chatManagerPtr->packetReceivedFromNetwork(in, receivedPacketSize, sessionId);
    }
    else{
        resourceStrings::print("Unexpected Packet Type Received in class serverLobby\r\n");
        //Only one packet is allowed from yetToBePromotedSession. if it is not name then remove the serverSession.
        if(yetToBePromotedSession.find(sessionId) != yetToBePromotedSession.end()){
            yetToBePromotedSession.erase(yetToBePromotedSession.find(sessionId));
            return;
        }
    }
    std::get<1>(players.find(sessionId)->second)->receiveMessage();
}

//excitedSessionId is the one to send state to and update of it to remaining
void serverLobby::managementJoin(int excitedSessionId, const std::string& playerNameFinal) {

    auto& playerSession = std::get<1>(players.find(excitedSessionId)->second);
    std::ostream& out = playerSession->out;

    //STEP 1;
    out.write(reinterpret_cast<const char*>(&constants::mtcLobby), sizeof(constants::mtcLobby));
    mtl t = mtl::SELFANDSTATE;
    out.write(reinterpret_cast<char*>(&t), sizeof(t));
    out.write(reinterpret_cast<char *>(&gameSelected), sizeof(gameSelected));

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
    playerSession->sendMessage();

    sendPLAYERJOINEDToAllExceptOne(excitedSessionId);
    resourceStrings::print("Player Joined: " + std::get<0>(players.find(excitedSessionId)->second) + "\r\n");
}


void serverLobby::sendPLAYERJOINEDToAllExceptOne(int excitedSessionId) {
    for(auto& player: players){
        if(player.first != excitedSessionId){
            auto& playerSession = std::get<1>(player.second);
            std::ostream& out = playerSession->out;

            //STEP 1;
            out.write(reinterpret_cast<const char*>(&constants::mtcLobby), sizeof(constants::mtcLobby));
            mtl t = mtl::PLAYERJOINED;
            out.write(reinterpret_cast<char*>(&t), sizeof(t));
            //STEP 2;
            auto mapPtr = players.find(excitedSessionId);
            int id = mapPtr->first;
            out.write(reinterpret_cast<char*>(&id), sizeof(id));
            //STEP 3;
            out << std::get<0>(mapPtr->second) << std::endl;

            playerSession->sendMessage();
        }
    }
}

void serverLobby::sendPLAYERLEFTToAllExceptOne(int excitedSessionId) {
    for(auto& player: players){
        auto& playerSession = std::get<1>(player.second);
        std::ostream& out = playerSession->out;

        //STEP 1;
        out.write(reinterpret_cast<const char*>(&constants::mtcLobby), sizeof(constants::mtcLobby));
        mtl t = mtl::PLAYERLEFT;
        out.write(reinterpret_cast<char*>(&t), sizeof(t));
        //STEP 2;
        out.write(reinterpret_cast<char*>(&excitedSessionId), sizeof(excitedSessionId));
        playerSession->sendMessage();
    }
}

void serverLobby::sendPLAYERLEFTDURINGGAMEToAllExceptOne(int excitedSessionId) {
    for(auto& player: players){
        auto& playerSession = std::get<1>(player.second);
        std::ostream& out = playerSession->out;

        //STEP 1;
        out.write(reinterpret_cast<const char*>(&constants::mtcLobby), sizeof(constants::mtcLobby));
        mtl t = mtl::PLAYERLEFTDURINGGAME;
        out.write(reinterpret_cast<char*>(&t), sizeof(t));
        //STEP 2;
        out.write(reinterpret_cast<char*>(&excitedSessionId), sizeof(excitedSessionId));

        playerSession->sendMessage();
    }
}
void serverLobby::setInputType(inputType type){
    sati::getInstance()->setInputType(type);
    inputTypeExpected = type;
}

void serverLobby::input(std::string inputString, inputType inputReceivedType){
    if(inputReceivedType == inputTypeExpected){
        if(inputReceivedType == inputType::SERVERLOBBYTWOORMOREPLAYERS){
            int input;
            if(constants::inputHelper(inputString, 1, 3,inputType::SERVERLOBBYTWOORMOREPLAYERS,
                                      inputType::SERVERLOBBYTWOORMOREPLAYERS, input)){
                if(input == 1){
                    //Start The Game
                    startTheGame();
                }else if(input == 2){
                    //Close Server
                    serverlistener.shutdown();
                    std::make_shared<home>(home(io))->run();
                }else{
                    //Exit
                    serverlistener.shutdown();
                }
            }
        }
        else if(inputReceivedType == inputType::OPTIONSELECTIONINPUTGAME){
            int input;
            if(constants::inputHelper(inputString, 2, 3, inputType::OPTIONSELECTIONINPUTGAME,
                                      inputType::OPTIONSELECTIONINPUTGAME, input)){
                if(input == 2){
                    serverlistener.shutdown();
                }else{
                    serverlistener.shutdown();
                    std::make_shared<home>(home(io))->run();
                }
            }
        }else{
            resourceStrings::print("No Handler For This InputType\r\n");
        }
    }else{
        resourceStrings::print("Message Of Unexpected Input Type Received\r\n");
    }
}

void serverLobby::gameExitFinished(){
    gameStarted = false;
    serverlistener.runAgain();

    if(gameSelected == constants::gamesEnum::GETAWAY){
        serverGetAwayPtr.reset();
    }else if(gameSelected == constants::gamesEnum::BLUFF){
        serverBluffPtr.reset();
    }
    if(players.size() == 1){
        serverlistener.registerForInputReceival();
        return;
    }
    if(serverOnly){
        inputTypeExpected = inputType::SERVERLOBBYTWOORMOREPLAYERS;
        PF::setLobbyMainTwoOrMorePlayers();
        sati::getInstance()->setBaseAndCurrentStateAndInputType(this,appState::LOBBY,
                                                                inputType::SERVERLOBBYTWOORMOREPLAYERS);
    }
}

void serverLobby::startTheGame(){
    //Start The Game
    serverlistener.shutdownAcceptorAndProbe();
    if(gameSelected == constants::gamesEnum::GETAWAY){
        serverGetAwayPtr = std::make_unique<serverGetAway>(players, *this);
    }else if(gameSelected == constants::gamesEnum::BLUFF){
        serverBluffPtr = std::make_unique<serverBluff>(players, *this);
    }
    gameStarted = true;
    if(serverOnly){
        setInputType(inputType::OPTIONSELECTIONINPUTGAME);
        sati::getInstance()->setBase(this, appState::GAME);
        PF::setGameMain();
    }
}
