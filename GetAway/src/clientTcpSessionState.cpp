#include "clientTcpSessionState.hpp"

#include <utility>
#include "clientLobbySession.hpp"


clientTcpSessionState::clientTcpSessionState(std::string playerName, std::string password) {
this->playerName = std::move(playerName);
this->password = std::move(password);
}

int clientTcpSessionState::getClassSendSize() const {
    return classSendSize;
}

void clientTcpSessionState::setClassSendSize(int size) {
    classSendSize = size;
}

