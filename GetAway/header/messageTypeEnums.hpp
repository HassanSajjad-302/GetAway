//
// Created by hassan on 10/30/20.
//

#ifndef GETAWAY_MESSAGETYPEENUMS_HPP
#define GETAWAY_MESSAGETYPEENUMS_HPP
#include <string>

struct messageTypes{
public:
    static const int STATE = 0;
    static const int PLAYERJOINED = 1;
    static const int PLAYERLEFT = 2;
    static const int MESSAGE = 3;
    static const int CHATMESSAGEID = 4;
    static const int GAMEFIRSTTURNSERVER= 5;
    static const int GAMETURNCLIENT = 6;
    static const int GAMETURNSERVER = 7;
    static inline std::string messageTypeStrings[8] = {"STATE", "PLAYERJOINED", "PLAYERLEFT", "CHATMESSAGE",
                                                       "CHATMESSAGEID", "GAMEFIRSTTURNSERVER", "GAMETURNCLIENT",
                                                       "GAMETURNSERVER"};
};

enum class lobbyMessageType{
    SELFANDSTATE = messageTypes::STATE,
    PLAYERJOINED = messageTypes::PLAYERJOINED,
    PLAYERLEFT = messageTypes::PLAYERLEFT,
    CHATMESSAGE = messageTypes::MESSAGE,
    CHATMESSAGEID = messageTypes::CHATMESSAGEID,
    GAMEFIRSTTURNSERVER = messageTypes::GAMEFIRSTTURNSERVER,
    GAMETURNCLIENT = messageTypes::GAMETURNCLIENT,
    GAMETURNSERVER = messageTypes::GAMETURNSERVER,
    ENUMSIZE = 8
};

enum class inputType{
    LOBBYINT = 0,
    MESSAGESTRING = 1,
    GAMEINT = 2,
    GAMEPERFORMTURN = 4,
    HOMEMAIN = 5,
    HOMEIPADDRESS = 6,
    HOMEPORTNUMBER = 7,
    HOMEASSIGNSERVERNAME = 8,
    HOMESELECTSERVER = 9,
    HOMECONNECTTINGOSERVER = 10
};
#endif //GETAWAY_MESSAGETYPEENUMS_HPP
