
#include "deckSuit.hpp"
#include <assert.h>
#include "constants.h"
Card::Card(deckSuit suit1, int cardNumber_) {
    assert((cardNumber_>= 0 && cardNumber_ < constants::SUITSIZE) && "Card is Not in range");
    suit = suit1;
    cardNumber = cardNumber_;
}

