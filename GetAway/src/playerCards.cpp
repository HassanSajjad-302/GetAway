//
// Created by hassan on 11/13/20.
//

#include <algorithm>
#include<cassert>
#include "playerCards.hpp"

playerCards::playerCards() {

}

void playerCards::addCard(int cardId) {
    assert(std::find(cards.begin(),cards.end(), cardId) == cards.end() &&
    "Card Is Already Present In The List");
    assert(cardId >=0 && cardId<52 && "Card Id is out-of-range");
    cards.push_back(cardId);
}

void playerCards::removeCard(int Id) {
    cards.remove(Id);
}

int playerCards::getCardCount() {
    return cards.size();
}

const std::list<int> &playerCards::getCards() {
    return cards;
}
