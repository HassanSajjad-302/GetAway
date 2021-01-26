#ifndef GETAWAY_CLIENTAUTH_HPP
#define GETAWAY_CLIENTAUTH_HPP

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include "session.hpp"
#include "asio/io_context.hpp"

//class clientAuthenticationManager;
class clientAuth
{
    std::string password;
    std::string playerName;
    std::shared_ptr<session<clientAuth>> authSession;
    asio::io_context& io;
public:
    explicit
    clientAuth(std::string playerName, std::string password, asio::io_context& io_);
    ~clientAuth();
    void join(std::shared_ptr<session<clientAuth>> authSession_);
    void starting();
};

#endif //GETAWAY_CLIENTAUTH_HPP
