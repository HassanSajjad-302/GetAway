//
// Created by hassan on 12/3/20.
//

#include "constants.h"
#include <assert.h>
int constants::cardsCount(const std::map<deckSuit, std::set<int>> &cards) {
    int count = 0;

    for(auto & cardPair: cards){
        count += cardPair.second.size();
    }
    assert(cards.size() < DECKSIZE && "cardSize has been passed more than DECKSIZE");
    return count;
}

void constants::initializeCards(std::map<deckSuit, std::set<int>>& cards){
    std::set<int> s;
    for(int i=0;i<4;++i){
        cards.emplace(static_cast<deckSuit>(i), s);
    }
}