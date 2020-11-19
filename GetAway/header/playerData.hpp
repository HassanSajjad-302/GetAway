//
// Created by hassan on 11/13/20.
//

#ifndef GETAWAY_PLAYERDATA_HPP
#define GETAWAY_PLAYERDATA_HPP

#include<list>
#include <vector>
#include "messageTypeEnums.hpp"
enum class turnType{
    TURNBYPLAYER,
    TURNNOOTHERPOSSIBLE,
    TURNPLAYEROFFLINE //There should be 2. player offline and player left. //And also player rejoining capability.
};

struct playerData{
    int id;
    std::list<int> cards;
    turnType playerTurnType;
    std::vector<lobbyMessageType> messageTypeExpectedGame;
    int cardValueAuto; //if playerTurnType == turnType::TURNNOOTHERPOSSIBLE, then this should be assigned.
    explicit playerData(int id_);
};

struct playerDataAndTurnManagement{
    std::vector<playerData> gamePlayers;
    int currentIndex;
};

#endif //GETAWAY_PLAYERDATA_HPP
