#ifndef GETAWAY_CLIENTAUTHMANAGER_HPP
#define GETAWAY_CLIENTAUTHMANAGER_HPP

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include "session.hpp"

//class clientAuthenticationManager;
class clientAuthManager
{
    std::string password;
    std::string playerName;
    std::shared_ptr<session<clientAuthManager>> authSession;

public:
    explicit
    clientAuthManager(std::string playerName, std::string password);
    ~clientAuthManager();
    void join(std::shared_ptr<session<clientAuthManager>> authSession_);
    void starting();
};

#endif //GETAWAY_CLIENTAUTHMANAGER_HPP
