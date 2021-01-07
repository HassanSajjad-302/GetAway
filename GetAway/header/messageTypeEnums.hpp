
#ifndef GETAWAY_MESSAGETYPEENUMS_HPP
#define GETAWAY_MESSAGETYPEENUMS_HPP
#include <string>

//message type core
enum class mtc{
    ROOM = 1,
    MESSAGE = 11,
    GAME = 12
};

//message type room
enum class mtr{
    SELFANDSTATE = 21,
    PLAYERJOINED  = 22,
    PLAYERLEFT = 23,
    PLAYERLEFTDURINGGAME = 24
};

//message type game
enum class mtg{
    GAMEFIRSTTURNSERVER = 31,
    GAMETURNCLIENT  = 32,
    GAMETURNSERVER  = 33,
};

enum class messageType{
    CHATMESSAGE  = 41,
    CHATMESSAGEID  = 42,
};
#endif //GETAWAY_MESSAGETYPEENUMS_HPP
