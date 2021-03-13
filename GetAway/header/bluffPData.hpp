
#ifndef GETAWAY_BLUFFPDATA_HPP
#define GETAWAY_BLUFFPDATA_HPP
#include <map>
#include <set>
#include<list>
#include <vector>
#include "deckSuit.hpp"
#include "constants.hpp"

//using this instead of map so that I can perform runtime debug checks with a clean interface.
struct bluffPData {
    int id;
    std::map <deckSuit, std::set<int>> cards;

    void insertCard(Card card);

    void removeCard(Card card);

    explicit bluffPData(int id_);
};


#endif //GETAWAY_BLUFFPDATA_HPP
