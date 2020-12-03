//
// Created by hassan on 11/21/20.
//

#ifndef GETAWAY_DECKSUIT_HPP
#define GETAWAY_DECKSUIT_HPP

//TODO
//Determine which one is not used and remove it
#include <string>
struct deckSuitValue{
public:
    static const int CLUB = 0;
    static const int HEART = 1;
    static const int SPADE = 2;
    static const int DIAMOND = 3;
    static const int FIRSTROUNDANY = 4;
    static const int ROUNDFIRSTTURN = 5;
    static const int ROUNDLASTTURN = 6;
    static const int THULLA = 7;
    static inline std::string displaySuitType[4] = {"CLUB", "HEART", "SPADE", "DIAMOND"};
    static inline std::string displayCards[13] = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
};

enum class deckSuit{
    CLUB = deckSuitValue::CLUB,
    HEART = deckSuitValue::HEART,
    SPADE = deckSuitValue::SPADE,
    DIAMOND = deckSuitValue::DIAMOND,
};

struct Card{
    deckSuit suit;
    int cardNumber;
    Card(deckSuit suit_, int cardNumber_);
};
#endif //GETAWAY_DECKSUIT_HPP
