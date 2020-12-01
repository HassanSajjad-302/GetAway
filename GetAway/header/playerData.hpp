//
// Created by hassan on 11/13/20.
//

#ifndef GETAWAY_PLAYERDATA_HPP
#define GETAWAY_PLAYERDATA_HPP

#include<list>
#include <vector>
#include "deckSuit.hpp"
enum class turnType{
    FIRSTROUNDSPADE,
    FIRSTROUNDANY,
    ROUNDFIRSTTURN,
    ROUNDMIDDLETURN,
    ROUNDLASTTURN,
    THULLA
};

struct playerData{
    int id;
    std::list<int> cards;
    bool turnExpected = false;
    turnType turnTypeExpected;
    explicit playerData(int id_);
};

#endif //GETAWAY_PLAYERDATA_HPP
