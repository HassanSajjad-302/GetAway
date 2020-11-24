#include "serverAuthManager.hpp"

#include <utility>
#include "messageTypeEnums.hpp"
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

std::istream &operator>>(std::istream &in, serverAuthManager &manager) {
    //STEP 1;
    //TODO
    char arr[61]; //This constant will be fed from somewhere else but one is added.
    in.getline(arr,61);
    std::string str(arr);
    if(str == manager.password)
    {
        //STEP 2;
        in.getline(arr,61);
        str = std::string(arr);
        manager.nextManager->setPlayerNameAdvanced(std::move(str));
        std::make_shared<session<serverLobbyManager, true>>(std::move(manager.serverAuthSessions.find(manager.excitedSessionId)->second->sock),
                                                            manager.nextManager)->registerSessionToManager();
        manager.serverAuthSessions.erase(manager.serverAuthSessions.find(manager.excitedSessionId));
    }
    else
    {
        manager.serverAuthSessions.erase(manager.serverAuthSessions.find(manager.excitedSessionId));
    }
    return in;
}

void serverAuthManager::leave(int id) {
    serverAuthSessions.erase(serverAuthSessions.find(id));
}
