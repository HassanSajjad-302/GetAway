//
// Created by hassan on 12/8/20.
//

#ifndef GETAWAY_CLIENTHOME_HPP
#define GETAWAY_CLIENTHOME_HPP

#include "asio/ip/tcp.hpp"
#ifdef ANDROID
#include "satiAndroid.hpp"
#else
#include "sati.hpp"
#endif
using namespace asio::ip;

class clientHome :inputRead, public std::enable_shared_from_this<clientHome>{

    asio::io_context& io;
    asio::executor_work_guard<decltype(io.get_executor())> guard;
    tcp::socket sock;
    inputType inputTypeExpected;
    std::string myName = "Player";
    std::string ipAddress;
    std::string portNumber;
    std::vector<std::tuple<std::string, std::string, std::string>> registeredServers;//ip-address, port-number, server-name
public:
    explicit clientHome(asio::io_context& io_);
    void run();
    void input(std::string inputString, inputType inputReceivedType) override;
    bool inputHelper(const std::string& inputString, int lower, int upper, inputType notInRange_,
                                 inputType invalidInput_, int& input_);
    void setInputType(inputType type);
    static int isValidIp4(const char *str);

    void CONNECTTOSERVERFail(asio::error_code ec);
    void promote();
};


#endif //GETAWAY_CLIENTHOME_HPP
