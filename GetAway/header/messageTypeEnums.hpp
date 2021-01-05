
#ifndef GETAWAY_MESSAGETYPEENUMS_HPP
#define GETAWAY_MESSAGETYPEENUMS_HPP
#include <string>

//message type core
enum class mtc{
    ROOM,
    MESSAGE,
    GAME
};

//message type room
enum class mtr{
    SELFANDSTATE ,
    PLAYERJOINED ,
    PLAYERLEFT
};

//message type game
enum class mtg{
    GAMEFIRSTTURNSERVER ,
    GAMETURNCLIENT ,
    GAMETURNSERVER ,
};

enum class messageType{
    CHATMESSAGE ,
    CHATMESSAGEID ,
    ENUMSIZE = 8
};
#endif //GETAWAY_MESSAGETYPEENUMS_HPP
