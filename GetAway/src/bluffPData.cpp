
#include <cassert>
#include "bluffPData.hpp"

bluffPData::bluffPData(int id_): id{id_} {
    std::set<int> s;
    for(int i=0; i<4; ++i){
        cards.emplace(static_cast<deckSuit>(i), s);
    }
}

void bluffPData::insertCard(Card card) {
    assert((card.cardNumber >= 0 && card.cardNumber < constants::DECKSIZE ) && "CardNumber Is Out Of Range");

#ifndef NDEBUG
    assert(cards.find(card.suit)->second.find(card.cardNumber) == cards.find(card.suit)->second.end() &&
           "Card Is Already Present In The Deck");
#endif
    cards.find(card.suit)->second.emplace(card.cardNumber);
}

void bluffPData::removeCard(Card card) {
    assert((card.cardNumber >= 0 && card.cardNumber < constants::DECKSIZE ) && "CardNumber Is Out Of Range");
#ifndef NDEBUG
    assert(cards.find(card.suit)->second.find(card.cardNumber) != cards.find(card.suit)->second.end() &&
           "Card Is Not Present In The Deck");
#endif
    cards.find(card.suit)->second.erase(cards.find(card.suit)->second.find(card.cardNumber));
}
