//
// Created by hassan on 10/31/20.
//

#include "serverGameManager.hpp"

std::ostream &operator<<(std::ostream &out, serverGameManager &state) {
    return out;
}

std::istream &operator>>(std::istream &in, serverGameManager &state) {
    return in;
}

int serverGameManager::join(std::shared_ptr<session<serverGameManager, true>> lobbySession) {
    return 0;
}

void serverGameManager::leave(int id) {

}
