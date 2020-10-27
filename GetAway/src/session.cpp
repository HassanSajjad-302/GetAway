//
// Created by hassan on 10/27/20.
//


#include <iostream>
#include <boost/asio.hpp>
#include <utility>
#include "session.hpp"
#include "Log_Macro.hpp"

namespace net = boost::asio;
using namespace net::ip;
using errorCode = boost::system::error_code;
//------------------------------------------------------------------------------

template <typename T>
session<T>::
session(
        tcp::socket socket,
        std::shared_ptr<T>  state)
        : sock(std::move(socket))
        , state(std::move(state))
{
}

template<typename T>
session<T>::~session() = default;

// Report a failure
template <typename T>
void
session<T>::
fail(errorCode ec, char const* what)
{
    // Don't report on canceled operations
    if(ec == net::error::operation_aborted)
        return;

    std::cerr << what << ": " << ec.message() << "\n";
}

//TODO
//This is a bug because if we are writing more than one message than
//
template <typename T>
void
session<T>::
sendMessage(void (T::*func)()) {
    out << *state;

    sendingMessagesQueue.emplace(lobbySessionStreamBuff.data());
    sendingMessageSizesQueue.emplace(lobbySessionStreamBuff.size());

    lobbySessionStreamBuff.consume(lobbySessionStreamBuff.size());

    std::vector<net::const_buffer> vec;
    vec.emplace_back(reinterpret_cast<char *>(&sendingMessageSizesQueue.front()), sizeof(sendingMessageSizesQueue.front()));
    vec.emplace_back(sendingMessagesQueue.front());
    net::async_write(sock, vec,
                     [self = shared_from_this<T>(), func](
                             errorCode ec, std::size_t bytes) {
                         self->sendingMessageSizesQueue.pop();
                         self->sendingMessagesQueue.pop();
                         self->state.*func();
                     });
}
template <typename T>
void
session<T>::
receiveMessage(void (T::*func)()) {
    //TODO
    //1500 is TCP MTU size. Provide it from somewhere else.
    sock.async_receive(lobbySessionStreamBuff.prepare(1500), [self = shared_from_this<T>(), func](
            errorCode ec, std::size_t bytes)
    {
        self->readMore(ec, bytes, func);
    });
}

template <typename T>
void
session<T>::
readMore(errorCode ec, int bytesRead, void (T::*func)()) {
    if(ec)
        return fail(ec, "write");

    lobbySessionStreamBuff.commit(bytesRead);
    int packetSize = 0;
    in.read(reinterpret_cast<char*>(packetSize), sizeof(packetSize));
    if(packetSize == bytesRead - 4){
        packetReceived(bytesRead, func);
    }
    else if(packetSize > bytesRead-4){
        //Start Composite Asynchronous Operation Before Reporting
        //The Received data.
        int remainingBytes = packetSize - (bytesRead - 4);
        net::async_read(sock, lobbySessionStreamBuff.prepare(remainingBytes),
                        [self = shared_from_this<T>(), bytesRead, remainingBytes, func](
                                errorCode ec, std::size_t bytes)
                        {
                            int consumingBytes = bytesRead + remainingBytes;
                            self->packetReceivedComposite(ec, bytes, consumingBytes, func);
                        });
    }
    else
    {
        //Why it read extra bytes
        std::cout<<"All guarantees are fucked up" <<std::endl;
        exit(-1);
    }
}

template <typename T>
void
session<T>::
packetReceived(int consumeBytes, void (T::*func)()) {
//Whole packet Received in One Call
    in >> *state;
    lobbySessionStreamBuff.consume(consumeBytes);
    state.*func();
}

template <typename T>
void
session<T>::
packetReceivedComposite(errorCode ec, int bytesRead, int consumingBytes, void (T::*func)()) {
    if(ec)
        return fail(ec, "write");

    packetReceived(consumingBytes, func);
}