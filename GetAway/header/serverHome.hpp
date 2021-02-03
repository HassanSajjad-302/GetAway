//
// Created by hassan on 12/9/20.
//

#ifndef GETAWAY_SERVERHOME_HPP
#define GETAWAY_SERVERHOME_HPP

#include "terminalInputBase.hpp"
#include "inputType.h"
#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"

using namespace asio::ip;

class serverHome : terminalInputBase, public std::enable_shared_from_this<serverHome>{
    class PF {
    public:
        static void setHomeMain();
        static void setHomeChangeServerName();
    };
    asio::io_context& io;
    asio::executor_work_guard<decltype(io.get_executor())> guard;
    tcp::socket sock;
    inputType inputTypeExpected;
    std::string myName = "Player";
    std::string ipAddress;
    std::vector<std::tuple<std::string, std::string, std::string>> registeredServers;//ip-address, port-number, server-name
    std::shared_ptr<serverHome> ref;
public:
    explicit serverHome(asio::io_context& io_);
    void run();
    void input(std::string inputString, inputType inputReceivedType) override;
    void setInputType(inputType type);
};


#endif //GETAWAY_SERVERHOME_HPP
