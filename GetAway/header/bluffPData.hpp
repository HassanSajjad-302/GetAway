
#ifndef GETAWAY_BLUFFPDATA_HPP
#define GETAWAY_BLUFFPDATA_HPP
#include <map>
#include <set>
#include<list>
#include <vector>
#include "deckSuit.hpp"
#include "constants.h"

struct bluffPData {
    int id;
    //std::list<int> cards;
    std::map <deckSuit, std::set<int>> cards;

    void insertCard(Card card);

    void removeCard(Card card);

    explicit bluffPData(int id_);
};


#endif //GETAWAY_BLUFFPDATA_HPP
