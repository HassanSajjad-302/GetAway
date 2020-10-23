#include "serverLobbySessionState.hpp"
#include "serverLobbySession.hpp"


serverLobbySessionState::
serverLobbySessionState(){
    classSendSize = 10; //gamePlayers.size() + \n + time_per_turn + \n
}
void
serverLobbySessionState::
join(serverLobbySession& session, const std::string& playerName)
{
    sessions.insert(&session);
    gamePlayers.push_back(playerName);
    classSendSize += playerName.size() + 1;
    session.id = gamePlayers.size()-1;
    session.player = playerName;
}

void
serverLobbySessionState::
leave(serverLobbySession& session)
{
    sessions.erase(&session);
    gamePlayers.erase(gamePlayers.begin() + session.id);
    classSendSize -= session.player.size();
    --classSendSize;
    int id = session.id;
    for(auto& sess: sessions){
        if(sess->id > id){
            --sess->id;
        }
    }
}


void
serverLobbySessionState::
stateSend(std::string message, serverLobbySession* session)
{
    auto const ss = std::make_shared<std::string const>(std::move(message));

    for(auto& sess: sessions)
    {
        if( sess == session){
            sess->sessionSend(ss);
        }
    }
}

int serverLobbySessionState::getClassSendSize() const {
    return classSendSize;
}

void serverLobbySessionState::setClassSendSize(int size) {
    classSendSize = size;
}
