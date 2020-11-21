//
// Created by hassan on 11/21/20.
//

#ifndef GETAWAY_DECKSUIT_HPP
#define GETAWAY_DECKSUIT_HPP

#include <string>
struct deckSuitValue{
public:
    static const int CLUB = 0;
    static const int HEART = 1;
    static const int SPADE = 2;
    static const int DIAMOND = 3;
    static inline std::string literal[4] = {"CLUB", "HEART", "SPADE", "DIAMOND"};
    static inline std::string displayCards[13] = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
};

enum class deckSuit{
    CLUB = deckSuitValue::CLUB,
    HEART = deckSuitValue::HEART,
    SPADE = deckSuitValue::SPADE,
    DIAMOND = deckSuitValue::DIAMOND,
    ENUMSIZE = 5
};

#endif //GETAWAY_DECKSUIT_HPP
