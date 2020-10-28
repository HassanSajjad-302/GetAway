#include "serverAuthenticationManager.hpp"

#include <utility>

serverAuthenticationManager::
serverAuthenticationManager(std::string password_, std::shared_ptr<serverListener> listener):
password(std::move(password_)), serverlistener(std::move(listener))
{
}

int serverAuthenticationManager::getClassSendSize() const {
    return classSendSize;
}

void serverAuthenticationManager::setClassSendSize(int size) {
    classSendSize = size;
}

int serverAuthenticationManager::getClassReceiveSize() const {
    return password.size() + nameLength + 2;
}

int serverAuthenticationManager::getMinimumReceivedBytes() const {
    return password.size() + 2;
}

std::string serverAuthenticationManager::getPlayerName() const {
    return playerName;
}

std::istream &operator>>(std::istream &in, serverAuthenticationManager &state) {
    //TODO
    char arr[61]; //This constant will be fed from somewhere else but one is added.
    in.getline(arr,61);
    std::string str(arr);
    if(str == state.password)
    {
        in.getline(arr,61);
        str = std::string(arr);
        state.playerName = std::move(str);
        state.passwordMatched = true;
    }
    else
    {
        state.passwordMatched = false;
    }
    return in;
}

void serverAuthenticationManager::authentication(tcp::socket sok) {
    if(passwordMatched){
        std::make_shared<sessionID<serverLobbyManager>>(std::move(sok), nextState)->sendMessage(&serverLobbyManager::customer);
    }
}