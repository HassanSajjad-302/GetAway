#include "clientLobbySession.hpp"
#include "Log_Macro.hpp"
#include <iostream>
#include <boost/asio.hpp>
#include <utility>
#include <clientLobbyManager.hpp>

namespace net = boost::asio;
using namespace net::ip;
using errorCode = boost::system::error_code;
//------------------------------------------------------------------------------

clientLobbySession::
    clientLobbySession(
        tcp::socket socket,
        std::shared_ptr<clientLobbyManager>  state)
    : sock(std::move(socket))
    , state(std::move(state))
{
}

void
clientLobbySession::
    run()
{
    // Read a request
    net::async_read_until(sock, lobbySessionStreamBuff, '\n',
                          [self = shared_from_this()]
                          (errorCode ec, std::size_t bytes)
                          {
          self->onRead(ec, bytes);
                          });
}

// Report a failure
void
clientLobbySession::
    fail(errorCode ec, char const* what)
{
    // Don't report on canceled operations
    if(ec == net::error::operation_aborted)
        return;

    std::cerr << what << ": " << ec.message() << "\n";
}

void
clientLobbySession::
    sessionSend(std::shared_ptr<std::string const> const& ss)
{
    net::async_write(
            sock,
        //net::buffer(*queue_.front()),
        net::buffer(*ss),
            [sp = shared_from_this()](
                errorCode ec, std::size_t bytes)
        {
            sp->onWrite(ec, bytes);
        });
}

void clientLobbySession::receiveMessage() {
    sock.async_receive(lobbySessionStreamBuff.prepare(32222), [self = shared_from_this()](
            errorCode ec, std::size_t bytes)
    {
        self->readMore(ec, bytes);
    });

}

void clientLobbySession::readMore(errorCode ec, int bytesRead) {
    if(ec)
        return fail(ec, "write");

    lobbySessionStreamBuff.commit(bytesRead);
    int packetSize = 0;
    is.read(reinterpret_cast<char*>(packetSize),sizeof(packetSize));
    if(packetSize == bytesRead - 4){
        packetReceived(bytesRead);
    }
    else if(packetSize > bytesRead-4){
        //Start Composite Asynchronous Operation Before Reporting
        //The Received data.
        int remainingBytes = packetSize - (bytesRead - 4);
        net::async_read(sock, lobbySessionStreamBuff.prepare(remainingBytes),
                        [self = shared_from_this(), bytesRead, remainingBytes](
                                errorCode ec, std::size_t bytes)
                        {
                            int consumingBytes = bytesRead + remainingBytes;
                            self->packetReceivedComposite(ec, bytes, consumingBytes);
                        });
    }
    else
        {
        //Why it read extra bytes
        std::cout<<"All guarantees are fucked up" <<std::endl;
        exit(-1);
    }
}

void clientLobbySession::packetReceived(int consumeBytes) {
//Whole packet Received in One Call
    messageType type;
    is.read(reinterpret_cast<char*>(type),sizeof(type));
    if(messageRequired){
        if(type != messageRequiredType){
            std::cout<<"Error. Unexpected Message Received"<<std::endl;
            exit(-1);
        }
    }
    if(type == messageType::STATE){
        is >> *state;
    }
    else if(type == messageType::CHAT_MESSAGE){

    }
    else if(type == messageType::UPDATE){

    }
    else{
        std::cout <<"Rethink Your Life Decisions" <<std::endl;
        exit(-1);
    }
    lobbySessionStreamBuff.consume(consumeBytes);
}

void clientLobbySession::packetReceivedComposite(errorCode ec, int bytesRead, int consumingBytes) {
    if(ec)
        return fail(ec, "write");

    packetReceived(consumingBytes);
}

//TODO
//This is a bug because if we are writing more than one message than
//
void clientLobbySession::sendMessage(messageType type){
    if(type == messageType::STATE){

    }
}
clientLobbySession::~clientLobbySession() = default;