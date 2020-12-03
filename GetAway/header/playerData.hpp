//
// Created by hassan on 11/13/20.
//

#ifndef GETAWAY_PLAYERDATA_HPP
#define GETAWAY_PLAYERDATA_HPP

#include <map>
#include <set>
#include<list>
#include <vector>
#include "deckSuit.hpp"
#include "constants.h"
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
    //std::list<int> cards;
    std::map<deckSuit, std::set<int>> cards;
    bool turnExpected = false;
    turnType turnTypeExpected;
    void insertCard(Card card);
    explicit playerData(int id_);
};

#endif //GETAWAY_PLAYERDATA_HPP
