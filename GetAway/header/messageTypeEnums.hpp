//
// Created by hassan on 10/30/20.
//

#ifndef GETAWAY_MESSAGETYPEENUMS_HPP
#define GETAWAY_MESSAGETYPEENUMS_HPP
#include <string>
enum class lobbyMessageType{
    STATE, UPDATE, MESSAGE
};
std::string lobbyMessageTypeStrings[3] = {"STATE", "UPDATE", "MESSAGE"};
#endif //GETAWAY_MESSAGETYPEENUMS_HPP
