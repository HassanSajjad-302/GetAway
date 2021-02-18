#include<memory>
#include <utility>
#include <home.hpp>
#include "serverListener.hpp"
#include "resourceStrings.hpp"
#include "constants.h"
#include "sati.hpp"

using errorCode = asio::error_code;


serverListener::
serverListener(
        asio::io_context& io_,
        const tcp::endpoint& endpoint,
        const std::string& serverName_)
        : acceptor(io_, endpoint)
        , tcpSock(io_)
        , probeListenerUdpSock(io_)
        , serverName(std::move(serverName_))
        , io{io_}
        , nextManager(*this, io)
{
    probeListenerEndpoint = udp::endpoint(udp::v4(), constants::PORT_PROBE_LISTENER);
}

void
serverListener::
run()
{
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
    ptr.reset();
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
        nextManager.newConnectionReceived(std::move(tcpSock));
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
        if(constants::inputHelper(inputString, 2, 3, inputType::SERVERLOBBYONEPLAYER,
                                  inputType::SERVERLOBBYONEPLAYER, input)){
            if(input == 2){
                //close server
                shutdown();
                std::make_shared<home>(home(io))->run();
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
    nextManager.shutDown();
    ptr.reset();
}

void serverListener::shutdownAcceptorAndProbe(){
    //This is assigned because otherwise this class will exit as from now on there will be no shared ptr holding it
    //as it was before being holded in lambdas of acceptr and probeListenerUdpSock which are now canceled and closed
    //Pointer will be reset in shutdown as we want to exit and in runAgain because now we again have reference pointing
    //to it
    ptr = shared_from_this();
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
