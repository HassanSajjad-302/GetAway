//
// Created by hassan on 12/2/20.
//

#ifndef GETAWAY_CONSTANTS_H
#define GETAWAY_CONSTANTS_H

#include <map>
#include <set>
#include "deckSuit.hpp"

namespace constants{
    constexpr int SUITSIZE = 1;
    constexpr int DECKSIZE = SUITSIZE * 4;

    int cardsCount(const std::map<deckSuit, std::set<int>>& cards);

    void initializeCards(std::map<deckSuit, std::set<int>>& cards);
}

#define LOG
#endif //GETAWAY_LOG_MACRO_HPP

#ifdef LOG
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#endif //GETAWAY_CONSTANTS_H
