//
// Created by hassan on 10/31/20.
//

#ifndef GETAWAY_SERVERGAMEMANAGER_HPP
#define GETAWAY_SERVERGAMEMANAGER_HPP

#include <memory>
#include <map>
#include <ostream>
#include "session.hpp"


class serverGameManager {
    std::map<int, std::tuple<const std::string,
            std::shared_ptr<session<serverGameManager, true>>>> gameData;

    friend std::ostream& operator<<(std::ostream& out, serverGameManager& state);
    friend std::istream& operator>>(std::istream& in, serverGameManager& state);


public:
    //Used-By-Session
    int join  (std::shared_ptr<session<serverGameManager, true>> lobbySession);
    int excitedSessionId;
    int receivedPacketSize;
    void leave (int id);



};


#endif //GETAWAY_SERVERGAMEMANAGER_HPP
