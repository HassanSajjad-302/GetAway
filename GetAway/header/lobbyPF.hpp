//
// Created by hassan on 12/7/20.
//

#ifndef GETAWAY_LOBBYPF_HPP
#define GETAWAY_LOBBYPF_HPP

#include <map>
#include <mutex>
#include <thread>
#include "messageTypeEnums.hpp"
#include "deckSuit.hpp"

class lobbyPF {
public:
    //input-statement-functions
    static void setInputStatementHome();
    static void setInputStatementHomeAccumulate();

    //others
    static void addOrRemovePlayer(const std::map<int, std::string>& gamePlayer_);
    static void addOrRemovePlayerAccumulate(const std::map<int, std::string>& gamePlayer_);
};


#endif //GETAWAY_LOBBYPF_HPP
