//
// Created by hassan on 10/30/20.
//

#ifndef GETAWAY_MESSAGETYPEENUMS_HPP
#define GETAWAY_MESSAGETYPEENUMS_HPP
#include <string>

struct messageTypes{
public:
    static const int STATE = 0;
    static const int UPDATE = 1;
    static const int MESSAGE = 2;
    static inline std::string messageTypeStrings[4] = {"STATE", "UPDATE", "CHAT_MESSAGE"};
};

enum class lobbyMessageType{
    SELFANDSTATE = messageTypes::STATE,
    UPDATE = messageTypes::UPDATE,
    CHATMESSAGE = messageTypes::MESSAGE,
    ENUMSIZE = 3
};
#endif //GETAWAY_MESSAGETYPEENUMS_HPP
