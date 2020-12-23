#include "serverAuthManager.hpp"
#include <utility>
#include "messageTypeEnums.hpp"
#include "constants.h"

serverAuthManager::
serverAuthManager(std::string password_, std::shared_ptr<serverListener> listener):
password(std::move(password_)),
serverlistener(std::move(listener))
{
    nextManager = std::make_shared<serverLobbyManager>(serverlistener);
}

int serverAuthManager::join(std::shared_ptr<session<serverAuthManager, true>> authSession) {
    authSession->receiveMessage();
    int id;
    if(serverAuthSessions.empty())
    {
        id = 0;
        serverAuthSessions.emplace(id, std::move(authSession));
    }
    else{
        id = serverAuthSessions.rbegin()->first + 1;
        serverAuthSessions.emplace(id, std::move(authSession));
    }
    return id;
}

void serverAuthManager::packetReceivedFromNetwork(std::istream &in, int receivedPacketSize, int excitedSessionId){
    //STEP 1;
    //TODO
    char arr[61]; //This constant will be fed from somewhere else but one is added.
    in.getline(arr,61);
    std::string str(arr);
    if(str == password)
    {
        //STEP 2;
        in.getline(arr,61);
        str = std::string(arr);
        nextManager->setPlayerNameAdvanced(std::move(str));
        std::make_shared<session<serverLobbyManager, true>>(std::move(serverAuthSessions.find(excitedSessionId)->second->sock),
                                                            nextManager)->registerSessionToManager();
        serverAuthSessions.erase(serverAuthSessions.find(excitedSessionId));
    }
    else
    {
        serverAuthSessions.erase(serverAuthSessions.find(excitedSessionId));
    }
}

void serverAuthManager::leave(int id) {
    serverAuthSessions.erase(serverAuthSessions.find(id));
}

void serverAuthManager::shutDown() {
    constants::Log("UseCount of nextManager from serverAuthManager {}", nextManager.use_count());
    nextManager->shutDown();
    constants::Log("UseCount of serverlistener from serverAuthManager {}", serverlistener.use_count());
    serverlistener.reset();
    nextManager.reset();
    for(auto& p: serverAuthSessions){
        constants::Log("UseCount of serverAuthSessions from serverAuthManager {}", std::get<1>(p).use_count());
        std::get<1>(p).reset();
    }
}
