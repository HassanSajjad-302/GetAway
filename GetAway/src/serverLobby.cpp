
#include <sati.hpp>
#include <serverHome.hpp>
#include "serverLobby.hpp"
#include "serverListener.hpp"
#include "serverChat.hpp"
serverLobby::serverLobby(serverListener& serverlistener_, asio::io_context &io_):
        serverlistener(serverlistener_), io{io_} {
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
    auto lobbySession = std::make_unique<session<serverLobby, true>>(std::move(sock), *this, maxID);
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
            sendPLAYERLEFTToAllExceptOne(id);
            resourceStrings::print("Player Left: " + std::get<0>(players.find(id)->second) + "\r\n");
            players.erase(players.find(id));
            if(players.size() == 1){
                serverlistener.registerForInputReceival();
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
            sati::getInstance()->setBase(this, appState::LOBBY);
            PF::setLobbyMainTwoOrMorePlayers();
            setInputType(inputType::SERVERLOBBYTWOORMOREPLAYERS);
        }
    }
    else if(messageTypeReceived == mtc::GAME){
        serverGetAwayPtr->packetReceivedFromNetwork(in, receivedPacketSize, sessionId);
    }else if(messageTypeReceived == mtc::MESSAGE){
        chatManagerPtr->packetReceivedFromNetwork(in, receivedPacketSize, sessionId);
    }
    else{
        resourceStrings::print("Unexpected Packet Type Received in class serverLobby\r\n");
        //Only one packet is allowed from yetToBePromotedSession. if it is not name then remove the session.
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
    out.write(reinterpret_cast<const char*>(&constants::mtcRoom), sizeof(constants::mtcRoom));
    mtr t = mtr::SELFANDSTATE;
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
            out.write(reinterpret_cast<const char*>(&constants::mtcRoom), sizeof(constants::mtcRoom));
            mtr t = mtr::PLAYERJOINED;
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
        if(player.first != excitedSessionId){
            auto& playerSession = std::get<1>(player.second);
            std::ostream& out = playerSession->out;

            //STEP 1;
            out.write(reinterpret_cast<const char*>(&constants::mtcRoom), sizeof(constants::mtcRoom));
            mtr t = mtr::PLAYERLEFT;
            out.write(reinterpret_cast<char*>(&t), sizeof(t));
            auto mapPtr = players.find(excitedSessionId);
            int id = mapPtr->first;
            //STEP 2;
            out.write(reinterpret_cast<char*>(&id), sizeof(id));

            playerSession->sendMessage();
        }
    }
}

void serverLobby::sendPLAYERLEFTDURINGGAMEToAllExceptOne(int excitedSessionId) {
    for(auto& player: players){
        auto& playerSession = std::get<1>(player.second);
        std::ostream& out = playerSession->out;

        //STEP 1;
        out.write(reinterpret_cast<const char*>(&constants::mtcRoom), sizeof(constants::mtcRoom));
        mtr t = mtr::PLAYERLEFTDURINGGAME;
        out.write(reinterpret_cast<char*>(&t), sizeof(t));
        auto mapPtr = players.find(excitedSessionId);
        int id = mapPtr->first;
        //STEP 2;
        out.write(reinterpret_cast<char*>(&id), sizeof(id));

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
                    serverlistener.shutdownAcceptorAndProbe();
                    serverGetAwayPtr = std::make_unique<serverGetAway>(players, *this);
                    gameStarted = true;
                    setInputType(inputType::OPTIONSELECTIONINPUTGAME);
                    sati::getInstance()->setBase(this, appState::GAME);
                    PF::setGameMain();
                }else if(input == 2){
                    //Close Server
                    serverlistener.shutdown();
                    std::make_shared<serverHome>(serverHome(io))->run();
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
                    std::make_shared<serverHome>(serverHome(io))->run();
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

    serverGetAwayPtr.reset();
    if(players.size() == 1){
        serverlistener.registerForInputReceival();
        return;
    }
    inputTypeExpected = inputType::SERVERLOBBYTWOORMOREPLAYERS;
    PF::setLobbyMainTwoOrMorePlayers();
    sati::getInstance()->setBaseAndCurrentStateAndInputType(this, appState::LOBBY,
                                                            inputType::SERVERLOBBYTWOORMOREPLAYERS);
}
