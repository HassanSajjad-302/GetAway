//
// Created by hassan on 10/30/20.
//

#ifndef GETAWAY_MESSAGETYPEENUMS_HPP
#define GETAWAY_MESSAGETYPEENUMS_HPP
#include <string>
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
