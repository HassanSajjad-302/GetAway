//
// Created by hassan on 12/9/20.
//

#ifndef GETAWAY_SERVERHOME_HPP
#define GETAWAY_SERVERHOME_HPP


#ifdef ANDROID
#include "satiAndroid.hpp"
#else
#include "sati.hpp"
#endif#include "asio/io_context.hpp"
#include "asio/ip/tcp.hpp"

using namespace asio::ip;

class serverHome :inputRead, public std::enable_shared_from_this<serverHome>{

    asio::io_context& io;
    asio::executor_work_guard<decltype(io.get_executor())> guard;
    tcp::socket sock;
    inputType inputTypeExpected;
    std::string myName = "Player";
    std::string ipAddress;
    std::vector<std::tuple<std::string, std::string, std::string>> registeredServers;//ip-address, port-number, server-name
public:
    explicit serverHome(asio::io_context& io_);
    void run();
    void input(std::string inputString, inputType inputReceivedType) override;
    bool inputHelper(const std::string& inputString, int lower, int upper, inputType notInRange_,
                     inputType invalidInput_, int& input_);
    void setInputType(inputType type);
};


#endif //GETAWAY_SERVERHOME_HPP
