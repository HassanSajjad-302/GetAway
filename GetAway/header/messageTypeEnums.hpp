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
    static inline std::string messageTypeStrings[7] = {"STATE", "PLAYERJOINED", "PLAYERLEFT", "CHATMESSAGE",
                                                       "CHATMESSAGEID", "GAMEFIRSTTURNSERVER", "GAMETURNCLIENT"};
};

enum class lobbyMessageType{
    SELFANDSTATE = messageTypes::STATE,
    PLAYERJOINED = messageTypes::PLAYERJOINED,
    PLAYERLEFT = messageTypes::PLAYERLEFT,
    CHATMESSAGE = messageTypes::MESSAGE,
    CHATMESSAGEID = messageTypes::CHATMESSAGEID,
    GAMEFIRSTTURNSERVER = messageTypes::GAMEFIRSTTURNSERVER,
    GAMETURNCLIENT = messageTypes::GAMETURNCLIENT,
    ENUMSIZE = 7
};

enum class inputType{
    LOBBYINT,
    LOBBYSTRING,
    GAMEINT,
    GAMESTRING
};
#endif //GETAWAY_MESSAGETYPEENUMS_HPP
