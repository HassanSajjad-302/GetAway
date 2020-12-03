//
// Created by hassan on 11/13/20.
//

#include <cassert>
#include "playerData.hpp"

playerData::playerData(int id_): id{id_} {
    std::set<int> s;
    for(int i=0; i<4; ++i){
        cards.emplace(static_cast<deckSuit>(i), s);
    }
}

void playerData::insertCard(Card card) {
    assert((card.cardNumber >= 0 && card.cardNumber < constants::DECKSIZE ) && "CardNumber Is Out Of Range");

#ifndef NDEBUG
    assert(cards.find(card.suit)->second.find(card.cardNumber) == cards.find(card.suit)->second.end() &&
    "Card Is Already Present In The Deck");
#endif
    cards.find(card.suit)->second.emplace(card.cardNumber);
}
