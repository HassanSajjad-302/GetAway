#include "clientAuth.hpp"
#include <utility>
#include "clientGetAway.hpp"
#include "session.hpp"

clientAuth::clientAuth(std::string playerName_, std::string password_, asio::io_context& io_) :
playerName(std::move(playerName_)), password(std::move(password_)), io{io_}
{
}

void clientAuth::starting() {
    tcp::socket tmp = std::move(authSession->sock);
    authSession.reset();
    std::make_shared<session<clientLobby>>(std::move(tmp),
                                           std::make_shared<clientLobby>(io))->registerSessionToManager();
}

namespace clientAuthManagerJoin{
    clientAuth* authManager;
    void func(){
        authManager->starting();
    }
}

void clientAuth::join(std::shared_ptr<session<clientAuth>> authSession_) {
    authSession = std::move(authSession_);
    authSession->out << password << std::endl;
    authSession->out << playerName << std::endl;
    clientAuthManagerJoin::authManager = this;
    authSession->sendMessage(clientAuthManagerJoin::func);
}

clientAuth::~clientAuth() = default;
