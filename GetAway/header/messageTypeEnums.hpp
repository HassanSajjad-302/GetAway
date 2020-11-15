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
    static const int HAND= 5;
    static inline std::string messageTypeStrings[6] = {"STATE", "PLAYERJOINED", "PLAYERLEFT", "CHATMESSAGE",
                                                       "CHATMESSAGEID", "HAND"};
};

enum class lobbyMessageType{
    SELFANDSTATE = messageTypes::STATE,
    PLAYERJOINED = messageTypes::PLAYERJOINED,
    PLAYERLEFT = messageTypes::PLAYERLEFT,
    CHATMESSAGE = messageTypes::MESSAGE,
    CHATMESSAGEID = messageTypes::CHATMESSAGEID,
    FIRSTGAMEMESSAGE = messageTypes::HAND,

    ENUMSIZE = 6
};



struct gameTypes{
public:
    static const int HAND = 0;
    static inline std::string messageTypeStrings[1] = {"HAND"};
};

enum class enumGameType{
    HAND = gameTypes::HAND,
    ENUMSIZE = 1
};

#endif //GETAWAY_MESSAGETYPEENUMS_HPP
