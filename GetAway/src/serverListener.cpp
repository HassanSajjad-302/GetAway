#include<memory>
#include <utility>
#include <home.hpp>
#include "serverListener.hpp"
#include "resourceStrings.hpp"
#include "constants.hpp"
#include "sati.hpp"
#include "clientLobby.hpp"

using errorCode = std::error_code;


serverListener::
serverListener(
        asio::io_context& io_,
        const tcp::endpoint& acceptorEndpoint_,
        std::string  serverName_,
        constants::gamesEnum gameSelected_)
        : acceptorEndpoint(acceptorEndpoint_),
        acceptor(io_, acceptorEndpoint_)
        , tcpSockAcceptor(io_)
        , probeListenerUdpSock(io_)
        , serverName(std::move(serverName_))
        , io{io_}
        , nextManager(*this, io, true, gameSelected_)
        , tcpSockClient(io)
{
    probeListenerEndpoint = udp::endpoint(udp::v4(), constants::PORT_PROBE_LISTENER);
}

serverListener::
serverListener(
        asio::io_context& io_,
        const tcp::endpoint& acceptorEndpoint_,
        std::string  serverName_,
        std::string clientName_,
        constants::gamesEnum gameSelected_)
        : acceptorEndpoint(acceptorEndpoint_),
        acceptor(io_, acceptorEndpoint_)
        , tcpSockAcceptor(io_)
        , probeListenerUdpSock(io_)
        , serverName(std::move(serverName_))
        , io{io_}
        , nextManager(*this, io, false, gameSelected_)
        , tcpSockClient(io)
        , clientName(std::move(clientName_))
        , gameSelected(gameSelected_)
{
    serverOnly = false;
    probeListenerEndpoint = udp::endpoint(udp::v4(), constants::PORT_PROBE_LISTENER);
}
void
serverListener::
run()
{
    // Start accepting a connection
    acceptor.async_accept(
            tcpSockAcceptor,
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


    if(!serverOnly){
        tcpSockClient.async_connect(tcp::endpoint(make_address_v4("127.0.0.1"), constants::PORT_CLIENT_CONNECTOR),
                                    [self = shared_from_this()](errorCode ec){
                                        if(ec){
                                            self->tcpSockClientConnectToServerFail(ec);
                                        }else{
                                            self->promote();
                                        }
                                    });
    }


    if(serverOnly){
        PF::setLobbyMainOnePlayer();
        sati::getInstance()->setBase(this, appState::LOBBY);
        sati::getInstance()->setInputType(inputType::SERVERLOBBYONEPLAYER);
    }
}

void serverListener::promote(){
    auto clientLobbySession = std::make_shared<clientSession<clientLobby, asio::io_context&, std::string,
    serverListener*, bool, constants::gamesEnum>>(
            std::move(tcpSockClient), io, std::move(clientName), this, false, gameSelected);
    clientPtr = clientLobbySession.get();
    clientPtr->run();
}

void serverListener::tcpSockClientConnectToServerFail(errorCode ec){
    resourceStrings::print("Could Not Connect tcpSockClient To Server. Error " + ec.message() + "\r\n");
    exit(-1);
}
void serverListener::runAgain(){
    // Start accepting a connection
    acceptor.open(acceptorEndpoint.protocol());
    acceptor.set_option(asio::socket_base::reuse_address(true));
    acceptor.bind(acceptorEndpoint);
    acceptor.listen();
    acceptor.async_accept(
            tcpSockAcceptor,
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
        tcpSockAcceptor.set_option(asio::ip::tcp::no_delay(true));   // enable PSH
        // Launch a new serverSession for this connection
        nextManager.newConnectionReceived(std::move(tcpSockAcceptor));
    }


    // Accept another connection
    acceptor.async_accept(
            tcpSockAcceptor,
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
                closeAcceptorAndShutdown();
                std::make_shared<home>(home(io))->run();
            }else{
                //exit application
                closeAcceptorAndShutdown();
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

void serverListener::closeAcceptorAndShutdown(){
    acceptor.close();
    shutdown();
}

void serverListener::shutdown() {
    probeListenerUdpSock.close();
    nextManager.shutDown();
    ptr.reset();
}

void serverListener::shutdownAcceptorAndProbe(){
    //This is assigned because otherwise this class will exit as from now on there will be no shared ptr holding it
    //as it was before being holded in lambdas of accept and probeListenerUdpSock which are now canceled and closed
    //Pointer will be reset in shutdown as we want to exit and in runAgain because now we again have reference pointing
    //to it
    ptr = shared_from_this();
    acceptor.close();
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

void serverListener::startTheGame() {
    nextManager.startTheGame();
}
