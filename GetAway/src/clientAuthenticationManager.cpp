#include "clientAuthenticationManager.hpp"

clientAuthenticationManager::clientAuthenticationManager(std::string playerName_, std::string password_) :
playerName(playerName_), password(password_)
{
}

std::ostream &operator<<(std::ostream &out, clientAuthenticationManager &state) {
    out << state.password << std::endl;
    out << state.playerName << std::endl;
    return out;
}