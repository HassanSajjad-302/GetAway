//
// Created by hassan on 12/2/20.
//

#ifndef GETAWAY_CONSTANTS_H
#define GETAWAY_CONSTANTS_H

#include <map>
#include <set>
#include "deckSuit.hpp"

#if !defined(NDEBUG) && !defined(ANDROID)
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#endif
namespace constants{
    constexpr int SUITSIZE = 2;
    constexpr int DECKSIZE = SUITSIZE * 4;
    constexpr int PORT = 3000;

    int cardsCount(const std::map<deckSuit, std::set<int>>& cards);

    void initializeCards(std::map<deckSuit, std::set<int>>& cards);

    template<typename... T>
    inline void Log(const char*p, T... args){
#if !defined(NDEBUG) && !defined(ANDROID)
        spdlog::info(p, args...);
#endif
    }
}

#endif