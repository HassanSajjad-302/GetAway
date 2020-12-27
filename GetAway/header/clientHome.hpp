//
// Created by hassan on 12/8/20.
//

#ifndef GETAWAY_CLIENTHOME_HPP
#define GETAWAY_CLIENTHOME_HPP

#include "asio/ip/tcp.hpp"
#include "asio/ip/udp.hpp"
#include "asio/steady_timer.hpp"
#include "inputType.h"
#include "terminalInputBase.hpp"
#include "constants.h"

using namespace asio::ip;

class clientHome : terminalInputBase, public std::enable_shared_from_this<clientHome>{

    asio::io_context& io;
    asio::executor_work_guard<decltype(io.get_executor())> guard;
    tcp::socket tcpSock;
    inputType inputTypeExpected;
    std::string myName = "Player";
    std::string ipAddress;
    std::vector<std::tuple<std::string, std::string>> addedServers;//server-name, ip-address

    //Following Are Used For broadcast server finding
    udp::socket broadcastudpSock;
    udp::endpoint senderEndpoint{asio::ip::address_v4::broadcast(), constants::PORT_PROBE_LISTENER};
    std::string emptybroadcastMessage = "";
    asio::steady_timer broadcastTimer;

    std::vector<std::tuple<std::string, std::string>> broadcastServersObtained;//server-name, ip-address
    udp::socket udpSock;
    udp::endpoint remoteEndpoint{};
    char receiveBuffer[512];

    std::shared_ptr<clientHome> ref;
public:
    explicit clientHome(asio::io_context& io_);
    void run();
    void input(std::string inputString, inputType inputReceivedType) override;
    void setInputType(inputType type);
    static int isValidIp4(const char *str);

    void CONNECTTOSERVERFail(asio::error_code ec);
    void promote();

    void broadcastResponseRecieved(std::error_code ec, size_t bytes);

    void startProbeBroadcast();
};


#endif //GETAWAY_CLIENTHOME_HPP
