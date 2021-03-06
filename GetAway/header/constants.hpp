
#ifndef GETAWAY_CONSTANTS_HPP
#define GETAWAY_CONSTANTS_HPP

#include <map>
#include <set>
#include "deckSuit.hpp"
#include "inputType.hpp"
#include "messageTypeEnums.hpp"

#if !defined(NDEBUG) && !defined(ANDROID)
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#endif
//For your game Foo, increase the value of constants::NUMBER_OF_GAMES and define another value FOO in gamesEnum
namespace constants{
    constexpr int SUITSIZE = 13;
    constexpr int DECKSIZE = SUITSIZE * 4;
    constexpr int PORT_SERVER_LISTENER = 3000;
    constexpr int PORT_CLIENT_CONNECTOR = PORT_SERVER_LISTENER;
    constexpr int PORT_PROBE_LISTENER = 3000;
    constexpr int PORT_PROBE_REPLY_LISTENER = 3001;
    constexpr int TCP_PACKET_MTU = 1500;
    constexpr int NUMBER_OF_GAMES = 2;
    enum class gamesEnum{GETAWAY, BLUFF };
    std::string const gamesNames[NUMBER_OF_GAMES] = {"GetAway", "Bluff"};

    const std::string gameRules = R"foo(
GAME RULES:


DECK OF SHUFFLED CARDS IS DISTRIBUTED AMONG PLAYERS. IF CAN'T BE DISTRIBUTED EQUALLY, SOME RANDOMLY SELECTED PLAYERS WILL GET AN ADDITIONAL CARD. THE NUMBER OF PLAYERS CAN'T BE LESS THAN 2.

IN THE FIRST ROUND, THERE IS NO TURN SEQUENCE. A PLAYER HAVING THE ACE OF SPADE MUST PLAY IT AND WILL BE THE PLAYER HAVING THE FIRST TURN IN THE NEXT ROUND. EACH PLAYER HAS TO PLAY A CARD. IF A PLAYER HAS A SPADE, THEN THAT MUST BE PLAYED. THESE CARDS WILL BE FLUSHED.

NOW EVERY PLAYER WILL PLAY ON THEIR TURN. THE PLAYER WHO PLAYS FIRST CAN PLAY ANY CARD AND SETS THE SUIT OF THE ROUND BY PLAYING THE CARD OF THAT SUIT. EVERY PLAYER HAS TO PLAY THE CARD OF THE SAME SUIT. THE TURN SEQUENCE IS RANDOMLY DETERMINED AND FOLLOWED FOR ALL SUBSEQUENT ROUNDS. NEXT PLAYERS ARE FREE TO PLAY ANY OF THE CARDS OF THE SUIT OF THE ROUND IF THEY HAVE MORE THAN ONE CARD OF THAT SUIT.

    IF ANY SUBSEQUENT PLAYER DOES NOT HAVE THE CARD OF THAT SUIT, THEN THAT PLAYER CAN PLAY ANY CARD AND SUBSEQUENT PLAYERS WON'T PLAY IN THAT ROUND. THE ROUND WILL BE TERMINATED BY FLUSHING THAT CARD WITH OTHER CARDS ON THE TABLE TO THE PLAYER WHO PLAYED THE HIGHEST CARD. THE PLAYER WHO HAD THE HIGHEST CARD WILL PLAY FIRST IN THE NEXT ROUND. HIGHEST CARD IS DETERMINED USING THE ORDER A > K > Q > J > 10 > 9 > 8 > 7 > 6 > 5 > 4 > 3 > 2.
    IF ALL PLAYERS PLAYED THE CARD OF THE SUIT, THEN THIS COLLECTION WILL BE FLUSHED AT THE END OF THE ROUND. THE PLAYER WHO HAD THE HIGHEST CARD WILL PLAY FIRST IN THE NEXT ROUND.

THE GAME ENDS WHEN ONE OF THE FOLLOWING HAPPENS.
1) ALL PLAYERS PLAYED THEIR CARDS. IN THIS CASE, NOBODY LOSES THE GAME.
2) ONE PLAYER WITH ONE OR MORE CARDS IS LEFT. IN THAT CASE, THE PLAYER WITH ONE OR MORE CARDS LOSES THE GAME.



NOTE:

A PLAYER CAN NOT LEAVE AMID THE GAME EVEN IF SHE HAS FINISHED HER CARDS. APPLICATION SHUTS IF THIS HAPPENS.
)foo";
    const std::string about = "VERSION 0.1";

    int cardsCount(const std::map<deckSuit, std::set<int>>& cards);

    void initializeCards(std::map<deckSuit, std::set<int>>& cards);

    constexpr mtc mtcLobby = mtc::LOBBY;
    constexpr mtc mtcGame = mtc::GAME;
    constexpr mtc mtcMessage = mtc::MESSAGE;
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