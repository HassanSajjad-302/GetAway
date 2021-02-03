#include<memory>
#include <utility>
#include <serverHome.hpp>
#include "serverListener.hpp"
#include "session.hpp"
#include "serverAuthManager.hpp"
#include "resourceStrings.hpp"
#include "constants.h"
#include "sati.hpp"

using errorCode = asio::error_code;


serverListener::
serverListener(
        asio::io_context& io_,
        const tcp::endpoint& endpoint,
        const std::string& serverName_,
        std::string password_)
        : acceptor(io_, endpoint)
        , tcpSock(io_)
        , probeListenerUdpSock(io_)
        , serverName(std::move(serverName_))
        , password(std::move(password_))
        , io{io_}
{
    probeListenerEndpoint = udp::endpoint(udp::v4(), constants::PORT_PROBE_LISTENER);
}

void
serverListener::
run()
{
    nextManager = std::make_shared<serverAuthManager>(std::move(password), shared_from_this(), io);
    // Start accepting a connection
    acceptor.async_accept(
            tcpSock,
            [self = shared_from_this()](errorCode ec)
        {
            self->onAccept(ec);
        });

    probeListenerUdpSock.open(udp::v4());
    probeListenerUdpSock.bind(probeListenerEndpoint);
    probeListenerUdpSock.async_receive_from(
            asio::buffer(recieveBuffer), remoteEndpoint,
            [self = shared_from_this()](errorCode ec, std::size_t bytesReceived)
    {
        self->probeReply(ec, bytesReceived);
    });
    PF::setLobbyMainOnePlayer();
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

    probeListenerUdpSock.open(udp::v4());
    probeListenerUdpSock.bind(probeListenerEndpoint);
    probeListenerUdpSock.async_receive_from(
            asio::buffer(recieveBuffer), remoteEndpoint,
            [self = shared_from_this()](errorCode ec, std::size_t bytesReceived)
            {
                self->probeReply(ec, bytesReceived);
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
        int input;
        if(constants::inputHelper(inputString, 2, 3, inputType::SERVERLOBBYONEPLAYER, inputType::SERVERLOBBYONEPLAYER, input)){
            if(input == 2){
                //close server
                shutdown();
                std::make_shared<serverHome>(serverHome(io))->run();
            }else{
                //exit application
                shutdown();
            }
        }
    }
    else{
        resourceStrings::print("Unexpected input type input received\r\n");
    }
}

void serverListener::registerForInputReceival() {
    PF::setLobbyMainOnePlayer();

    sati::getInstance()->setBaseAndCurrentStateAndInputType(this, appState::LOBBY,
                                                            inputType::SERVERLOBBYONEPLAYER);
}

void serverListener::shutdown() {
    acceptor.cancel();
    probeListenerUdpSock.close();
    nextManager->shutDown();
    constants::Log("UseCount of nextManager from serverListener {}", nextManager.use_count());
    nextManager.reset();
}

void serverListener::shutdownAcceptorAndProbe(){
    acceptor.cancel();
    probeListenerUdpSock.close();
}

void serverListener::probeReply(asio::error_code ec, int size) {
    if(ec)
        return fail(ec, "probeReply");
    else{
        asio::error_code error;
        asio::ip::udp::socket sock(acceptor.get_executor());

        sock.open(asio::ip::udp::v4(), error);
        if (!error)
        {
            sock.send_to(asio::const_buffer(serverName.c_str(), serverName.size()),
                         udp::endpoint{remoteEndpoint.address(), constants::PORT_PROBE_REPLY_LISTENER});
            sock.close(error);
        }
    }

    //For Another probe
    probeListenerUdpSock.async_receive_from(
            asio::buffer(recieveBuffer), remoteEndpoint,
            [self = shared_from_this()](errorCode ec, std::size_t bytesReceived)
            {
                self->probeReply(ec, bytesReceived);
            });
}
