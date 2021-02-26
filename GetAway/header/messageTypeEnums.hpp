
#ifndef GETAWAY_MESSAGETYPEENUMS_HPP
#define GETAWAY_MESSAGETYPEENUMS_HPP
#include <string>

//message type core
enum class mtc{
    LOBBY = 1,
    MESSAGE = 11,
    GAME = 12
};

//message type lobby
enum class mtl{
    PLAYERNAME = 20,
    SELFANDSTATE = 21,
    PLAYERJOINED  = 22,
    PLAYERLEFT = 23,
    PLAYERLEFTDURINGGAME = 24,
};

//message type game getaway
enum class mtgg{
    GAMEFIRSTTURNSERVER = 31,
    GAMETURNCLIENT  = 32,
    GAMETURNSERVER  = 33,
};

//message type game bluff
enum class mtgb{
    SERVER_CLIENTTURNNORMAL,
    SERVER_CLIENTTURNPASS,
    SERVER_CLIENTTURNCHECK,
    CLIENT_TURNPASS,
    CLIENT_TURNNORMAL,
    CLIENT_TURNCHECK
};


enum class messageType{
    CHATMESSAGE  = 41,
    CHATMESSAGEID  = 42,
};
#endif //GETAWAY_MESSAGETYPEENUMS_HPP
