#include "clientAuthManager.hpp"

#include <utility>
#include "clientLobbyManager.hpp"
#include "session.hpp"
clientAuthManager::clientAuthManager(std::string playerName_, std::string password_) :
playerName(std::move(playerName_)), password(std::move(password_))
{
}

std::ostream &operator<<(std::ostream &out, clientAuthManager &state) {
    out << state.password << std::endl;
    out << state.playerName << std::endl;
    return out;
}

void clientAuthManager::starting() {
    tcp::socket tmp = std::move(authSession->sock);
    authSession.reset();
    std::make_shared<session<clientLobbyManager>>(std::move(tmp),
                                                 std::make_shared<clientLobbyManager>())->registerSessionToManager();
}

void clientAuthManager::join(std::shared_ptr<session<clientAuthManager>> authSession_) {
    authSession = std::move(authSession_);
    authSession->sendMessage(&clientAuthManager::starting);
}

clientAuthManager::~clientAuthManager() {
}
