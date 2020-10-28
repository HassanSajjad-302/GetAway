#ifndef GETAWAY_CLIENTAUTHENTICATIONMANAGER_HPP
#define GETAWAY_CLIENTAUTHENTICATIONMANAGER_HPP

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <iostream>


class clientAuthenticationManager
{
    std::string password;
    std::string playerName;

    friend std::ostream& operator<<(std::ostream& out, clientAuthenticationManager& state);

public:
    explicit
    clientAuthenticationManager(std::string playerName, std::string password);

};

#endif //GETAWAY_CLIENTAUTHENTICATIONMANAGER_HPP
