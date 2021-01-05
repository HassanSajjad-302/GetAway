#include "clientAuthManager.hpp"
#include <utility>
#include "clientLobbyManager.hpp"
#include "session.hpp"

clientAuthManager::clientAuthManager(std::string playerName_, std::string password_, asio::io_context& io_) :
playerName(std::move(playerName_)), password(std::move(password_)), io{io_}
{
}

void clientAuthManager::starting() {
    tcp::socket tmp = std::move(authSession->sock);
    authSession.reset();
    std::make_shared<session<clientLobbyManager>>(std::move(tmp),
                                                 std::make_shared<clientRoomManager>(io))->registerSessionToManager();
}

void clientAuthManager::join(std::shared_ptr<session<clientAuthManager>> authSession_) {
    authSession = std::move(authSession_);
    authSession->out << password << std::endl;
    authSession->out << playerName << std::endl;
    authSession->sendMessage(&clientAuthManager::starting);
}

clientAuthManager::~clientAuthManager() = default;
