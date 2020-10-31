#ifndef GETAWAY_CLIENTAUTHENTICATIONMANAGER_HPP
#define GETAWAY_CLIENTAUTHENTICATIONMANAGER_HPP

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <iostream>
#include "boost/asio.hpp"
#include "session.hpp"
namespace net = boost::asio;
using namespace net::ip;

//class clientAuthenticationManager;
class clientAuthenticationManager
{
    std::string password;
    std::string playerName;
    std::shared_ptr<session<clientAuthenticationManager>> authSession;
    friend std::ostream& operator<<(std::ostream& out, clientAuthenticationManager& state);

public:
    explicit
    clientAuthenticationManager(std::string playerName, std::string password);
    void join(std::shared_ptr<session<clientAuthenticationManager>> authSession_);
    void starting();
};

#endif //GETAWAY_CLIENTAUTHENTICATIONMANAGER_HPP
