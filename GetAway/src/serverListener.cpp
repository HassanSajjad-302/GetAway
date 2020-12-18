#include<iostream>
#include<memory>
#include<system_error>
#include <utility>
#include "serverListener.hpp"
#include "session.hpp"
#include "serverAuthManager.hpp"
#include "serverPF.hpp"
namespace net = boost::asio;
using namespace net::ip;
using errorCode = boost::system::error_code;


serverListener::
serverListener(
    net::io_context& ioc,
    const tcp::endpoint& endpoint,
    std::string password_)
    : acceptor(ioc, endpoint)
    , sock(ioc)
    , password(std::move(password_))
{
}

void
serverListener::
run()
{
    nextManager = std::make_shared<serverAuthManager>(std::move(password), shared_from_this());
    // Start accepting a connection
    acceptor.async_accept(
            sock,
            [self = shared_from_this()](errorCode ec)
        {
            self->onAccept(ec);
        });

    serverPF::setLobbyMainOnePlayer();
    sati::getInstance()->setBase(this, appState::LOBBY);
    sati::getInstance()->setInputType(inputType::SERVERLOBBYONEPLAYER);
}

void serverListener::runAgain(){
    // Start accepting a connection
    acceptor.async_accept(
            sock,
            [self = shared_from_this()](errorCode ec)
            {
                self->onAccept(ec);
            });
}

// Report a failure
void
serverListener::
fail(errorCode ec, char const* what)
{
    // Don't report on canceled operations
    if(ec == net::error::operation_aborted)
        return;
    std::cerr << what << ": " << ec.message() << "\n";
}

// Handle a connection
void
serverListener::
onAccept(errorCode ec)
{
    if(ec)
        return fail(ec, "accept");
    else{
        sock.set_option(boost::asio::ip::tcp::no_delay(true));   // enable PSH
        // Launch a new session for this connection
        std::make_shared<session<serverAuthManager,true>>(
                std::move(sock),
                nextManager)->registerSessionToManager();
    }


    // Accept another connection
    acceptor.async_accept(
            sock,
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
            std::cout<<"Wrong Input\r\n";
            sati::getInstance()->setInputType(inputType::SERVERLOBBYONEPLAYER);
        }
    }
    else{
        std::cout<<"Unexpected input type input received\r\n";
    }
}

void serverListener::registerForInputReceival() {
    serverPF::setLobbyMainOnePlayer();
    sati::getInstance()->setBase(this, appState::LOBBY);
}

void serverListener::shutdown() {
    acceptor.cancel();
    nextManager->shutDown();
    spdlog::info("UseCount of nextManager from serverListener {}", nextManager.use_count());
    nextManager.reset();
}

void serverListener::shutdownAcceptor(){
    acceptor.cancel();
}