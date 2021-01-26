
#include "clientHome.hpp"
#include "constants.h"
#include "clientAuth.hpp"
#include <memory>
#include <regex>
#include <chrono>
#include <clientHomePF.hpp>
#include "resourceStrings.hpp"
#include "sati.hpp"

clientHome::clientHome(asio::io_context &io_): io(io_), guard(io_.get_executor()), tcpSock(io_), udpSock(io_),
                                               broadcastudpSock(io), broadcastTimer(io){

}

void clientHome::run() {
    sati::getInstance()->setBase(this, appState::HOME);
    setInputType(inputType::HOMEMAIN);
    PF::setInputStatementMAIN();
    ref = this->shared_from_this();
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
            if(constants::inputHelper(inputString,
                           1, 6, inputType::HOMEMAIN, inputType::HOMEMAIN, input)){
                if(input == 1){
                    //Add Server
                    PF::setInputStatementIPADDRESS();
                    setInputType(inputType::HOMEIPADDRESS);
                }else if(input == 2){
                    //Join Server
                    if(addedServers.empty()){
                        setInputType(inputType::HOMEMAIN);
                        resourceStrings::print("No Registered Servers\r\n");
                    }else{
                        PF::setInputStatementSELECTSERVER(addedServers);
                        setInputType(inputType::HOMEJOINSERVER);
                    }
                }else if(input == 3){
                    //Find Local Server
                    PF::setInputStatementHome7R3(broadcastServersObtained);
                    //BroadCast
                    asio::error_code error;

                    broadcastudpSock.open(asio::ip::udp::v4(), error);
                    if (error){

                    }
                    broadcastudpSock.set_option(asio::socket_base::broadcast(true));
                    startProbeBroadcast();
                    //udp server for response recording of the udp broadcast
                    udp::endpoint host_endpoint{udp::v4(), constants::PORT_PROBE_REPLY_LISTENER};
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
                    PF::setInputStatementHome7R3(broadcastServersObtained);
                }else if(input == 4){
                    //Game Rules
                    PF::setInputStatementHomeGameRules();
                    setInputType(inputType::HOMEGAMERULES);
                }else if(input == 5){
                    //About
                    PF::setInputStatementHomeAbout();
                    setInputType(inputType::HOMEABOUT);
                }else if(input == 6){
                    //Exit
                    if(tcpSock.is_open()){
                        asio::error_code ec;
                        tcpSock.shutdown(tcp::socket::shutdown_both, ec);
                        tcpSock.close();
                    }
                    guard.reset();
                    ref.reset();
                }
            }
        }else if(inputReceivedType == inputType::HOMEIPADDRESS){
            if(inputString.empty()){
                PF::setInputStatementMAIN();
                setInputType(inputType::HOMEMAIN);
            }else{
                if(isValidIp4(inputString.c_str())){
                    ipAddress = std::move(inputString);
                    setInputType(inputType::HOMEASSIGNSERVERNAME);
                    PF::setInputStatementASSIGNSERVERNAME();
                }else{
                    resourceStrings::print("Please enter valid ip address\r\n");
                    setInputType(inputType::HOMEIPADDRESS);
                }
            }
        }else if(inputReceivedType == inputType::HOMEASSIGNSERVERNAME){
            if(inputString.empty()){
                PF::setInputStatementMAIN();
                setInputType(inputType::HOMEMAIN);
            }else{
                addedServers.emplace_back(std::move(inputString), std::move(ipAddress));
                PF::setInputStatementMAIN();
                setInputType(inputType::HOMEMAIN);
            }
        }else if(inputReceivedType == inputType::HOMEJOINSERVER){
            int input;
            assert(!addedServers.empty());
            if(constants::inputHelper(inputString, 1, addedServers.size(), inputType::HOMEJOINSERVER,
                                      inputType::HOMEJOINSERVER, input)){
                --input;
                PF::setInputStatementClientName();
                setInputType(inputType::HOMECLIENTNAME);
                connectWithServer = addedServers[input];
            }
        }else if(inputReceivedType == inputType::HOMECONNECTTINGTOSERVER){
            int input;
            if(constants::inputHelper(inputString, 1, 1,inputType::HOMECONNECTTINGTOSERVER,
                           inputType::HOMECONNECTTINGTOSERVER,input)){
                asio::error_code ec;
                tcpSock.shutdown(tcp::socket::shutdown_both, ec);
                tcpSock.close();
            }
        }else if(inputReceivedType == inputType::HOMECONNECTTOPROBEREPLYSERVER){
            if(inputString.empty()){
                //move back
                setInputType(inputType::HOMEMAIN);
                PF::setInputStatementHome7();
                //close probeListenerUdpSock server
                udpSock.close();
                broadcastServersObtained.clear();
            }else{
                int input;
                if(constants::inputHelper(inputString, 1, broadcastServersObtained.size(),inputType::HOMECONNECTTOPROBEREPLYSERVER,
                               inputType::HOMECONNECTTOPROBEREPLYSERVER,input)){
                    setInputType(inputType::HOMECONNECTTINGTOSERVER);
                    broadcastTimer.cancel();
                    broadcastudpSock.close();
                    udpSock.close();
                    --input;
                    PF::setInputStatementClientName();
                    setInputType(inputType::HOMECLIENTNAME);
                    connectWithServer = broadcastServersObtained[input];
                }
            }
        }else if(inputReceivedType == inputType::HOMECLIENTNAME){
            if(!inputString.empty()){
                myName = inputString;
            }
            tcpSock.async_connect(tcp::endpoint(make_address_v4(std::get<1>(connectWithServer)),
                                                constants::PORT_CLIENT_CONNECTOR),
                                  [self = this](asio::error_code ec){
                                      if(ec){
                                          self->CONNECTTOSERVERFail(ec);
                                      }else{
                                          self->promote();
                                      }
                                  });
            PF::setInputStatementConnectingToServer(std::get<0>(connectWithServer));
            setInputType(inputType::HOMECONNECTTINGTOSERVER);
        }else if(inputReceivedType == inputType::HOMEGAMERULES){
            PF::setInputStatementMAIN();
            setInputType(inputType::HOMEMAIN);
        }else if(inputReceivedType == inputType::HOMEABOUT){
            PF::setInputStatementMAIN();
            setInputType(inputType::HOMEMAIN);
        }
    }else{
        resourceStrings::print("Unexpected input type input received\r\n");
    }
}

void clientHome::CONNECTTOSERVERFail(asio::error_code ec){
    resourceStrings::print(ec.message() + "\r\n");
    PF::setInputStatementMAIN();
    setInputType(inputType::HOMEMAIN);
    resourceStrings::print("Connect To Server Failed\r\n");
}

void clientHome::setInputType(inputType type) {
    sati::getInstance()->setInputType(type);
    inputTypeExpected = type;
}

void clientHome::promote() {
    std::make_shared<session<clientAuth>>(std::move(tcpSock), std::make_shared<clientAuth>(
            std::move(myName), "password", io))->registerSessionToManager();
    guard.reset();
    ref.reset();
}

void clientHome::startProbeBroadcast(){
    broadcastudpSock.send_to(asio::const_buffer(emptybroadcastMessage.c_str(),
                                                emptybroadcastMessage.size()), senderEndpoint);
    broadcastTimer.expires_from_now(std::chrono::milliseconds(500));
    broadcastTimer.async_wait([self = this](asio::error_code ec){
        if(ec == asio::error::operation_aborted){
            return;
        }
        if(!ec){
            self->startProbeBroadcast();
        }else{
            resourceStrings::print(std::string("startProbeBroadcast") + ": " + ec.message() + "\r\n");
        }
    });
}

void clientHome::broadcastResponseRecieved(errorCode ec, std::size_t bytes){

    auto it = std::find_if(broadcastServersObtained.begin(), broadcastServersObtained.end(), [&end = this->remoteEndpoint](auto& p){
        return std::get<1>(p) == end.address().to_string();
    });
    if(it != broadcastServersObtained.end()){
        return;
    }
    broadcastServersObtained.emplace_back(std::string(receiveBuffer, receiveBuffer + bytes), remoteEndpoint.address().to_string());
    PF::setInputStatementHome7R3(broadcastServersObtained);
    //
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