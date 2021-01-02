
#ifndef GETAWAY_MESSAGETYPEENUMS_HPP
#define GETAWAY_MESSAGETYPEENUMS_HPP
#include <string>

//message type core
enum class mtc{
    ROOM,
    MESSAGE,
    GAME
};

enum class adf{

};
namespace adf{

}
enum class messageType{
    SELFANDSTATE ,
    PLAYERJOINED ,
    PLAYERLEFT ,
    CHATMESSAGE ,
    CHATMESSAGEID ,
    GAMEFIRSTTURNSERVER ,
    GAMETURNCLIENT ,
    GAMETURNSERVER ,
    ENUMSIZE = 8
};
#endif //GETAWAY_MESSAGETYPEENUMS_HPP
