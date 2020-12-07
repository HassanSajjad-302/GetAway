//
// Created by hassan on 12/7/20.
//

#ifndef GETAWAY_LOBBYPF_HPP
#define GETAWAY_LOBBYPF_HPP

#include <set>
#include <mutex>
#include <thread>
#include "boost/asio.hpp"
#include "messageTypeEnums.hpp"
#include "deckSuit.hpp"

class lobbyPF {
public:
    //USED ONLY IN LOBBY

    //input-statement-functions
    static void setInputStatementHome();
    static void setInputStatementHomeAccumulate();

    //others
    static void addOrRemovePlayer(const std::map<int, std::string>& gamePlayer_);
    static void addOrRemovePlayerAccumulate(const std::map<int, std::string>& gamePlayer_);
};


#endif //GETAWAY_LOBBYPF_HPP
