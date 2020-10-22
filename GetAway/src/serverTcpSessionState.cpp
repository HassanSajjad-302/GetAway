#include "serverTcpSessionState.hpp"

#include <utility>

serverTcpSessionState::
serverTcpSessionState(std::string password){
    this->password = std::move(password);
    classSendSize = 10; //gamePlayers.size() + \n + time_per_turn + \n
}

int serverTcpSessionState::getClassSendSize() const {
    return classSendSize;
}

void serverTcpSessionState::setClassSendSize(int size) {
    classSendSize = size;
}

int serverTcpSessionState::getClassReceiveSize() const {
    return password.size() + nameLength + 2;
}

int serverTcpSessionState::getMinimumReceivedBytes() const {
    return password.size() + 2;
}

bool serverTcpSessionState::getPasswordMatched() const {
    return passwordMatched;
}
