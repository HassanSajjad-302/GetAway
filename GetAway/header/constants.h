
#ifndef GETAWAY_CONSTANTS_H
#define GETAWAY_CONSTANTS_H

#include <map>
#include <set>
#include "deckSuit.hpp"
#include "inputType.h"

#if !defined(NDEBUG) && !defined(ANDROID)
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#endif
namespace constants{
    constexpr int SUITSIZE = 2;
    constexpr int DECKSIZE = SUITSIZE * 4;
    constexpr int PORT_SERVER_LISTENER = 3000;
    constexpr int PORT_CLIENT_CONNECTOR = PORT_SERVER_LISTENER;
    constexpr int PORT_PROBE_LISTENER = 3000;
    constexpr int PORT_PROBE_REPLY_LISTENER = 3001;
    constexpr int TCP_PACKET_MTU = 1500;
    int cardsCount(const std::map<deckSuit, std::set<int>>& cards);

    void initializeCards(std::map<deckSuit, std::set<int>>& cards);

    inline void exitCookedTerminal() {
#ifdef __linux__
        system("stty cooked");
#endif
    }

    bool inputHelper(const std::string& inputString, int lower, int upper, inputType notInRange_,
                     inputType invalidInput_, int& input_);

    template<typename... T>
    inline void Log(const char*p, T... args){
#if !defined(NDEBUG) && !defined(ANDROID)
        spdlog::info(p, args...);
#endif
    }
}

#endif