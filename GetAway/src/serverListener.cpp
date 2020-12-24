#include<memory>
#include <utility>
#include "serverListener.hpp"
#include "session.hpp"
#include "serverAuthManager.hpp"
#include "serverPF.hpp"
#include "resourceStrings.hpp"
#include "constants.h"
using errorCode = asio::error_code;


serverListener::
serverListener(
        asio::io_context& ioc,
        const tcp::endpoint& endpoint,
        const std::string& serverName_,
        std::string password_)
        : acceptor(ioc, endpoint)
        , tcpSock(ioc)
        , udpSock(ioc)
        , serverName(std::move(serverName_))
        , password(std::move(password_))
{
    hostEndpoint = udp::endpoint(udp::v4(), constants::PORT);
}

void
serverListener::
run()
{
    nextManager = std::make_shared<serverAuthManager>(std::move(password), shared_from_this());
    // Start accepting a connection
    acceptor.async_accept(
            tcpSock,
            [self = shared_from_this()](errorCode ec)
        {
            self->onAccept(ec);
        });

    udpSock.open(udp::v4());
    udpSock.bind(hostEndpoint);
    udpSock.async_receive_from(
            asio::buffer(recieveBuffer), remoteEndpoint,
            [self = shared_from_this()](errorCode ec, std::size_t bytesReceived)
    {
        self->broadcastReceivalHandler(ec, bytesReceived);
    });
    serverPF::setLobbyMainOnePlayer();
    sati::getInstance()->setBase(this, appState::LOBBY);
    sati::getInstance()->setInputType(inputType::SERVERLOBBYONEPLAYER);
}

void serverListener::runAgain(){
    // Start accepting a connection
    acceptor.async_accept(
            tcpSock,
            [self = shared_from_this()](errorCode ec)
            {
                self->onAccept(ec);
            });

    udpSock.open(udp::v4());
    udpSock.bind(hostEndpoint);
    udpSock.async_receive_from(
            asio::buffer(recieveBuffer), remoteEndpoint,
            [self = shared_from_this()](errorCode ec, std::size_t bytesReceived)
            {
                self->broadcastReceivalHandler(ec, bytesReceived);
            });

}

// Report a failure
void
serverListener::
fail(errorCode ec, char const* what)
{
    resourceStrings::print(std::string(what) + ": " + ec.message() + "\r\n");

    // Don't report on canceled operations
    if(ec == asio::error::operation_aborted)
        return;
}

// Handle a connection
void
serverListener::
onAccept(errorCode ec)
{
    if(ec)
        return fail(ec, "accept");
    else{
        tcpSock.set_option(asio::ip::tcp::no_delay(true));   // enable PSH
        // Launch a new session for this connection
        std::make_shared<session<serverAuthManager,true>>(
                std::move(tcpSock),
                nextManager)->registerSessionToManager();
    }


    // Accept another connection
    acceptor.async_accept(
            tcpSock,
            [self = shared_from_this()](errorCode ec)
        {
            self->onAccept(ec);
        });
}

void serverListener::input(std::string inputString, inputType inputReceivedType) {
    if(inputReceivedType == inputType::SERVERLOBBYONEPLAYER){
        if(inputString != "1"){
            //Exit The Game Here
            //TODO
        }else{
            resourceStrings::print("Wrong Input\r\n");
            sati::getInstance()->setInputType(inputType::SERVERLOBBYONEPLAYER);
        }
    }
    else{
        resourceStrings::print("Unexpected input type input received\r\n");
    }
}

void serverListener::registerForInputReceival() {
    serverPF::setLobbyMainOnePlayer();
    sati::getInstance()->setBase(this, appState::LOBBY);
}

void serverListener::shutdown() {
    acceptor.cancel();
    nextManager->shutDown();
    constants::Log("UseCount of nextManager from serverListener {}", nextManager.use_count());
    nextManager.reset();
}

void serverListener::shutdownAcceptor(){
    acceptor.cancel();
}

void serverListener::broadcastReceivalHandler(asio::error_code ec, int size) {
    if(ec)
        return fail(ec, "broadcastRecivalHandler");
    else{
        asio::error_code error;
        asio::ip::udp::socket sock(acceptor.get_executor());

        sock.open(asio::ip::udp::v4(), error);
        if (!error)
        {
            sock.send_to(asio::const_buffer(serverName.c_str(), serverName.size()), remoteEndpoint);
            sock.close(error);
        }
    }

    //For Another broadcast
    udpSock.async_receive_from(
            asio::buffer(recieveBuffer), remoteEndpoint,
            [self = shared_from_this()](errorCode ec, std::size_t bytesReceived)
            {
                self->broadcastReceivalHandler(ec, bytesReceived);
            });
}
