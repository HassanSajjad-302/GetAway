#include "serverAuthenticationManager.hpp"

#include <utility>

serverAuthenticationManager::
serverAuthenticationManager(std::string password_, std::shared_ptr<serverListener> listener):
password(std::move(password_)), serverlistener(std::move(listener))
{
}

void serverAuthenticationManager::join(std::shared_ptr<session<serverLobbyManager>>) {

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
        std::make_shared<sessionID<serverLobbyManager>>(std::move(sok), nextManager)->sendMessage(&serverLobbyManager::customer);
    }
}