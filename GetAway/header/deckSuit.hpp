
#ifndef GETAWAY_DECKSUIT_HPP
#define GETAWAY_DECKSUIT_HPP

#include <string>
struct deckSuitValue{
public:
    static inline std::string displaySuitType[4] = {"CLUB", "HEART", "SPADE", "DIAMOND"};
    static inline std::string displayCards[13] = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
};

enum class deckSuit{
    CLUB ,
    HEART ,
    SPADE ,
    DIAMOND ,
};

struct Card{
    deckSuit suit;
    int cardNumber;
    Card(deckSuit suit_, int cardNumber_);
};
#endif //GETAWAY_DECKSUIT_HPP
