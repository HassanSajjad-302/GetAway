#include "clientAuthenticationManager.hpp"

#include <utility>
#include "clientLobbyManager.hpp"
#include "session.hpp"
clientAuthenticationManager::clientAuthenticationManager(std::string playerName_, std::string password_) :
playerName(std::move(playerName_)), password(std::move(password_))
{
}

std::ostream &operator<<(std::ostream &out, clientAuthenticationManager &state) {
    out << state.password << std::endl;
    out << state.playerName << std::endl;
    return out;
}

void clientAuthenticationManager::starting() {
    std::make_shared<session<clientLobbyManager>>(std::move(authSession->sock),
                                                 std::make_shared<clientLobbyManager>());
}

void clientAuthenticationManager::join(std::shared_ptr<session<clientAuthenticationManager>> authSession_) {
    authSession = std::move(authSession_);
    authSession->sendMessage(&clientAuthenticationManager::starting);
}
