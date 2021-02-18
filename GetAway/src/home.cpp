
#include "home.hpp"
#include "constants.h"
#include "clientLobby.hpp"
#include "serverListener.hpp"
#include <memory>
#include <chrono>
#include "resourceStrings.hpp"
#include "sati.hpp"

home::home(asio::io_context &io_): io(io_), guard(io_.get_executor()), tcpSock(io_), udpSock(io_),
                                   broadcastudpSock(io), broadcastTimer(io){
}

//If i delete the following function and just use the constructor, then a behavior occurs that I can't reason about.
// If I exit pressing 6 in home, io.run() still blocks instead of exiting as there is no async operation being
// waited on. Another behavior that I can't reason about is that I can't declare a destructor in this class. It will
//be an error.
void home::run() {
    sati::getInstance()->setBase(this, appState::HOME);
    setInputType(inputType::HOMEMAIN);
    PF::setInputStatementMAIN();
    ref = this->shared_from_this();
}

int home::isValidIp4(const char *str) {
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

void home::input(std::string inputString, inputType inputReceivedType) {
    if(inputReceivedType == inputTypeExpected){
        if(inputReceivedType == inputType::HOMEMAIN){
            int input;
            if(constants::inputHelper(inputString,
                           1, 7, inputType::HOMEMAIN, inputType::HOMEMAIN, input)){
                if(input == 1){
                    //Change Server Name
                    PF::setHomeChangeServerName();
                    setInputType(inputType::HOMESTARTSERVER);
                }else if(input == 2){
                    //Add Server
                    PF::setInputStatementIPADDRESS();
                    setInputType(inputType::HOMEIPADDRESS);
                }else if(input == 3){
                    //Join Server
                    if(addedServers.empty()){
                        setInputType(inputType::HOMEMAIN);
                        resourceStrings::print("No Registered Servers\r\n");
                    }else{
                        PF::setInputStatementSELECTSERVER(addedServers);
                        setInputType(inputType::HOMEJOINSERVER);
                    }
                }else if(input == 4){
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
                            [self = this](std::error_code ec, std::size_t bytesReceived)
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
                }else if(input == 5){
                    //Game Rules
                    PF::setInputStatementHomeGameRules();
                    setInputType(inputType::HOMEGAMERULES);
                }else if(input == 6){
                    //About
                    PF::setInputStatementHomeAbout();
                    setInputType(inputType::HOMEABOUT);
                }else if(input == 7){
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
        }else if(inputReceivedType == inputType::HOMESTARTSERVER){
            if(inputString.empty()){
                std::make_shared<serverListener>(
                        io,
                        tcp::endpoint{tcp::v4(), constants::PORT_SERVER_LISTENER},
                        "Server")->run();
                guard.reset();
                ref.reset();
            }else{
                std::make_shared<serverListener>(
                        io,
                        tcp::endpoint{tcp::v4(), constants::PORT_SERVER_LISTENER},
                        inputString)->run();
                guard.reset();
                ref.reset();
            }
        }
        else if(inputReceivedType == inputType::HOMEIPADDRESS){
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

void home::CONNECTTOSERVERFail(asio::error_code ec){
    resourceStrings::print(ec.message() + "\r\n");
    PF::setInputStatementMAIN();
    setInputType(inputType::HOMEMAIN);
    resourceStrings::print("Connect To Server Failed\r\n");
}

void home::setInputType(inputType type) {
    sati::getInstance()->setInputType(type);
    inputTypeExpected = type;
}

void home::promote() {
    std::make_shared<clientSession<clientLobby, false, asio::io_context&, std::string>>(std::move(tcpSock), io,
            std::move(myName))->run();
    guard.reset();
    ref.reset();
}

void home::startProbeBroadcast(){
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

void home::broadcastResponseRecieved(errorCode ec, std::size_t bytes){

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
            [self = this](std::error_code ec, std::size_t bytesReceived)
            {
                if(ec)
                    resourceStrings::print(std::string("error in lambda broadcastResponse Receival") +
                                           ": " + ec.message() + "\r\n");
                else{
                    self->broadcastResponseRecieved(ec, bytesReceived);
                }
            });
}