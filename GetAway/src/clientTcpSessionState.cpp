#include "clientTcpSessionState.hpp"

#include <utility>
#include "clientLobbySession.hpp"


clientTcpSessionState::clientTcpSessionState(std::string playerName, std::string password) {
this->playerName = std::move(playerName);
this->password = std::move(password);
}

int clientTcpSessionState::getClassWriteSize() const {
    return password.size() + playerName.size() + 2;
}