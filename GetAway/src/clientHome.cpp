//
// Created by hassan on 12/8/20.
//

#include "clientHome.hpp"
#include "constants.h"
#include "clientAuthManager.hpp"
#include <memory>
#include <regex>
#include "resourceStrings.hpp"

clientHome::clientHome(asio::io_context &io_): io(io_), guard(io.get_executor()), tcpSock(io_), udpSock(io_){

}

void clientHome::run() {
    sati::getInstance()->setBase(this, appState::HOME);
    setInputType(inputType::HOMEMAIN);
    clientHomePF::setInputStatementMAIN();
}

int clientHome::isValidIp4(const char *str) {
    int segs = 0;   /* Segment count. */
    int chcnt = 0;  /* Character count within segment. */
    int accum = 0;  /* Accumulator for segment. */

    /* Catch NULL pointer. */

    if (str == nullptr)
        return 0;

    /* Process every character in string. */

    while (*str != '\0') {
        /* Segment changeover. */

        if (*str == '.') {
            /* Must have some digits in segment. */

            if (chcnt == 0)
                return 0;

            /* Limit number of segments. */

            if (++segs == 4)
                return 0;

            /* Reset segment values and restart loop. */

            chcnt = accum = 0;
            str++;
            continue;
        }
        /* Check numeric. */

        if ((*str < '0') || (*str > '9'))
            return 0;

        /* Accumulate and check segment. */

        if ((accum = accum * 10 + *str - '0') > 255)
            return 0;

        /* Advance other segment specific stuff and continue loop. */

        chcnt++;
        str++;
    }

    /* Check enough segments and enough characters in last segment. */

    if (segs != 3)
        return 0;

    if (chcnt == 0)
        return 0;

    /* Address okay. */

    return 1;
}

void clientHome::input(std::string inputString, inputType inputReceivedType) {
    if(inputReceivedType == inputTypeExpected){
        if(inputReceivedType == inputType::HOMEMAIN){
            int input;
            if(inputHelper(inputString,
                           1, 7, inputType::HOMEMAIN, inputType::HOMEMAIN, input)){
                if(input == 1){
                    //Add Server
                    clientHomePF::setInputStatementIPADDRESS();
                    setInputType(inputType::HOMEIPADDRESS);
                }else if(input == 2){
                    //Join Server
                    if(registeredServers.empty()){
                        setInputType(inputType::HOMEMAIN);
                        resourceStrings::print("No Registered Servers\r\n");
                    }else{
                        clientHomePF::setInputStatementSELECTSERVER(registeredServers);
                        setInputType(inputType::HOMESELECTSERVER);
                    }
                }else if(input == 3){
                    //Find Local Server
                    clientHomePF::setInputStatementHome7R3(broadcastServersObtained);
                    //BroadCast
                    asio::error_code error;
                    asio::ip::udp::socket broadcastudpSock(io);

                    broadcastudpSock.open(asio::ip::udp::v4(), error);
                    if (!error)
                    {
                        broadcastudpSock.bind(udp::endpoint(udp::v4(), constants::PORT));
                        broadcastudpSock.set_option(asio::socket_base::broadcast(true));
                        asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::broadcast(), constants::PORT);

                        std::string str = "";
                        broadcastudpSock.send_to(asio::const_buffer(str.c_str(), str.size()), senderEndpoint);
                        broadcastudpSock.close(error);
                    }
                    //udp server for response recording of the udp broadcast
                    udp::endpoint host_endpoint{udp::v4(), constants::PORT};
                    udpSock.open(udp::v4());
                    udpSock.bind(host_endpoint);
                    udpSock.async_receive_from(
                            asio::buffer(receiveBuffer), remoteEndpoint,
                            [self = this](errorCode ec, std::size_t bytesReceived)
                            {
                                if(ec)
                                    resourceStrings::print(std::string("error in lambda broadcastResponse Receival") +
                                    ": " + ec.message() + "\r\n");
                                else{
                                    self->broadcastResponseRecieved(ec, bytesReceived);
                                }
                            });
                    setInputType(inputType::HOMECONNECTTOPROBEREPLYSERVER);
                    clientHomePF::setInputStatementHome7R3(broadcastServersObtained);
                }else if(input == 4){
                    //Change Name
                }else if(input == 5){
                    //Game Rules
                }else if(input == 6){
                    //Liscence
                }else if(input == 7){
                    //Exit
                    if(tcpSock.is_open()){
                        asio::error_code ec;
                        tcpSock.shutdown(tcp::socket::shutdown_both, ec);
                        tcpSock.close();
                    }
                    guard.reset();
#ifdef __linux__
                    system("stty cooked");
#endif
                }
            }
        }else if(inputReceivedType == inputType::HOMEIPADDRESS){
            if(isValidIp4(inputString.c_str())){
                ipAddress = std::move(inputString);
                setInputType(inputType::HOMEASSIGNSERVERNAME);
                clientHomePF::setInputStatementASSIGNSERVERNAME();
            }else{
                resourceStrings::print("Please enter valid ip address\r\n");
                setInputType(inputType::HOMEIPADDRESS);
            }
        }else if(inputReceivedType == inputType::HOMEASSIGNSERVERNAME){
            registeredServers.emplace_back(std::move(ipAddress), std::move(inputString));
            clientHomePF::setInputStatementMAIN();
            setInputType(inputType::HOMEMAIN);
        }else if(inputReceivedType == inputType::HOMESELECTSERVER){
            int input;
            assert(!registeredServers.empty());
            if(inputHelper(inputString, 1, registeredServers.size(), inputType::HOMESELECTSERVER,
                           inputType::HOMESELECTSERVER, input)){
                --input;
                tcpSock.async_connect(tcp::endpoint(make_address_v4(std::get<0>(registeredServers[input])),
                                                    constants::PORT),
                                      [self = this](asio::error_code ec){
                    if(ec){
                        self->CONNECTTOSERVERFail(ec);
                    }else{
                        self->promote();
                    }
                });
                clientHomePF::setInputStatementConnectingToServer(std::get<0>(registeredServers[input]));
                setInputType(inputType::HOMECONNECTTINGTOSERVER);
            }
        }else if(inputReceivedType == inputType::HOMECONNECTTINGTOSERVER){
            int input;
            if(inputHelper(inputString, 1, 1,inputType::HOMECONNECTTINGTOSERVER,
                           inputType::HOMECONNECTTINGTOSERVER,input)){
                asio::error_code ec;
                tcpSock.shutdown(tcp::socket::shutdown_both, ec);
                tcpSock.close();
            }
        }else if(inputReceivedType == inputType::HOMECONNECTTOPROBEREPLYSERVER){
            if(inputString == ""){
                //move back
                setInputType(inputType::HOMEMAIN);
                clientHomePF::setInputStatementHome7();
                //close udpSock server
                udpSock.shutdown(asio::socket_base::shutdown_both);
                udpSock.close();
                broadcastServersObtained.clear();
            }else{
                int input;
                if(inputHelper(inputString, 1, broadcastServersObtained.size(),inputType::HOMECONNECTTOPROBEREPLYSERVER,
                               inputType::HOMECONNECTTOPROBEREPLYSERVER,input)){
                    --input;
                    tcpSock.async_connect(tcp::endpoint(make_address_v4(std::get<1>(broadcastServersObtained[input])),
                                                        constants::PORT),
                                          [self = this](asio::error_code ec){
                                              if(ec){
                                                  self->CONNECTTOSERVERFail(ec);
                                              }else{
                                                  self->promote();
                                              }
                                          });
                    clientHomePF::setInputStatementConnectingToServer(std::get<0>(broadcastServersObtained[input]));
                    setInputType(inputType::HOMECONNECTTINGTOSERVER);
                    udpSock.shutdown(asio::socket_base::shutdown_both);
                    udpSock.close();
                    broadcastServersObtained.clear();
                }
            }
        }
    }else{
        resourceStrings::print("Unexpected input type input received\r\n");
    }
}

void clientHome::CONNECTTOSERVERFail(asio::error_code ec){
    resourceStrings::print(ec.message() + "\r\n");
    clientHomePF::setInputStatementMAIN();
    setInputType(inputType::HOMEMAIN);
    resourceStrings::print("Connect To Server Failed\r\n");
}

bool clientHome::inputHelper(const std::string& inputString, int lower, int upper, inputType notInRange_,
                                     inputType invalidInput_, int& input_){
    try{
        int num = std::stoi(inputString);
        if(num>=lower && num<=upper){
            input_ = num;
            return true;
        }else{
            sati::getInstance()->accumulatePrint();
            sati::getInstance()->setInputType(notInRange_);
            resourceStrings::print("Please enter integer in range\r\n");
            return false;
        }
    }
    catch (std::invalid_argument& e) {
        sati::getInstance()->accumulatePrint();
        sati::getInstance()->setInputType(invalidInput_);
        resourceStrings::print("Invalid Input\r\n");
        return false;
    }
}

void clientHome::setInputType(inputType type) {
    sati::getInstance()->setInputType(type);
    inputTypeExpected = type;
}

void clientHome::promote() {
    std::make_shared<session<clientAuthManager>>(std::move(tcpSock), std::make_shared<clientAuthManager>(
            std::move(myName), "password"))->registerSessionToManager();
    guard.reset();
}

void clientHome::broadcastResponseRecieved(errorCode ec, std::size_t bytes){
    broadcastServersObtained.emplace_back(std::string(receiveBuffer, receiveBuffer + bytes), remoteEndpoint.address().to_string());
    clientHomePF::setInputStatementHome7R3(broadcastServersObtained);
    //
    udp::endpoint host_endpoint{udp::v4(), constants::PORT};
    udpSock.async_receive_from(
            asio::buffer(receiveBuffer), remoteEndpoint,
            [self = this](errorCode ec, std::size_t bytesReceived)
            {
                if(ec)
                    resourceStrings::print(std::string("error in lambda broadcastResponse Receival") +
                                           ": " + ec.message() + "\r\n");
                else{
                    self->broadcastResponseRecieved(ec, bytesReceived);
                }
            });
}