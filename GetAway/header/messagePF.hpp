//
// Created by hassan on 12/7/20.
//

#ifndef GETAWAY_MESSAGEPF_HPP
#define GETAWAY_MESSAGEPF_HPP

#include <string>

class messagePF {

public:

    static void setInputStatement();
    static void setInputStatementAccumulate();


    static void add(const std::string& playerName, const std::string& message);
    static void addAccumulate(const std::string& playerName, const std::string& message);
};


#endif //GETAWAY_MESSAGEPF_HPP
