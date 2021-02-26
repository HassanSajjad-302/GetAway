
#ifndef GETAWAY_GETAWAYPDATA_HPP
#define GETAWAY_GETAWAYPDATA_HPP

#include <map>
#include <set>
#include<list>
#include <vector>
#include "deckSuit.hpp"
#include "constants.h"
#include "clientBluff.hpp"

enum class turnType;
struct getAwayPData{
    int id;
    //std::list<int> cards;
    std::map<deckSuit, std::set<int>> cards;
    bool turnExpected = false;
    turnType turnTypeExpected;
    void insertCard(Card card);
    explicit getAwayPData(int id_);
};

#endif //GETAWAY_GETAWAYPDATA_HPP
