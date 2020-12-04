//
// Created by hassan on 12/2/20.
//

#ifndef GETAWAY_CONSTANTS_H
#define GETAWAY_CONSTANTS_H

#include <map>
#include <set>
#include "deckSuit.hpp"

namespace constants{
    constexpr int SUITSIZE = 3;
    constexpr int DECKSIZE = SUITSIZE * 4;

    int cardsCount(const std::map<deckSuit, std::set<int>>& cards);

    void initializeCards(std::map<deckSuit, std::set<int>>& cards);
}
#endif //GETAWAY_CONSTANTS_H
