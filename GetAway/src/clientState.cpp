#include "clientState.hpp"
#include "clientLobbySession.hpp"


clientState::
clientState(){
    classSendSize = 10; //gamePlayers.size() + \n + time_per_turn + \n
}
void
clientState::
join(clientLobbySession& session, const std::string& playerName)
{
    sessions.insert(&session);
    gamePlayers.push_back(playerName);
    classSendSize += playerName.size() + 1;
    session.id = gamePlayers.size()-1;
    session.player = playerName;
}

void
clientState::
leave(clientLobbySession& session)
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
clientState::
stateSend(std::string message, clientLobbySession* session)
{
    auto const ss = std::make_shared<std::string const>(std::move(message));

    for(auto& sess: sessions)
    {
        if( sess == session){
            sess->sessionSend(ss);
        }
    }
}

int clientState::getClassSendSize() const {
    return classSendSize;
}

void clientState::setClassSendSize(int size) {
    classSendSize = size;
}
