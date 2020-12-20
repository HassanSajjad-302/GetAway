#include "clientAuthManager.hpp"
#include <utility>
#include "clientLobbyManager.hpp"
#include "session.hpp"
#include "boost/asio/ip/tcp.hpp"

clientAuthManager::clientAuthManager(std::string playerName_, std::string password_) :
playerName(std::move(playerName_)), password(std::move(password_))
{
}

void clientAuthManager::starting() {
    tcp::socket tmp = std::move(authSession->sock);
    authSession.reset();
    std::make_shared<session<clientLobbyManager>>(std::move(tmp),
                                                 std::make_shared<clientLobbyManager>())->registerSessionToManager();
}

void clientAuthManager::join(std::shared_ptr<session<clientAuthManager>> authSession_) {
    authSession = std::move(authSession_);
    authSession->out << password << std::endl;
    authSession->out << playerName << std::endl;
    authSession->sendMessage(&clientAuthManager::starting);
}

clientAuthManager::~clientAuthManager() = default;
