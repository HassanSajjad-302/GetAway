#ifndef GETAWAY_CLIENTAUTHMANAGER_HPP
#define GETAWAY_CLIENTAUTHMANAGER_HPP

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include "session.hpp"
#include "asio/io_context.hpp"

//class clientAuthenticationManager;
class clientAuthManager
{
    std::string password;
    std::string playerName;
    std::shared_ptr<session<clientAuthManager>> authSession;
    asio::io_context& io;
public:
    explicit
    clientAuthManager(std::string playerName, std::string password, asio::io_context& io_);
    ~clientAuthManager();
    void join(std::shared_ptr<session<clientAuthManager>> authSession_);
    void starting();
};

#endif //GETAWAY_CLIENTAUTHMANAGER_HPP
