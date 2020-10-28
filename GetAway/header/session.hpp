//
// Created by hassan on 10/27/20.
//

#ifndef GETAWAY_SESSION_HPP
#define GETAWAY_SESSION_HPP

#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <boost/asio.hpp>


namespace net = boost::asio;
using namespace net::ip;
using errorCode = boost::system::error_code;

template <typename T>
class session : public std::enable_shared_from_this<T>
{
    tcp::socket sock;
    std::shared_ptr<T> state;
    boost::asio::streambuf sessionStreamBuff;
    std::ostream out{&sessionStreamBuff};
    std::istream in{&sessionStreamBuff};
    std::queue<net::const_buffer> sendingMessagesQueue;
    std::queue<int> sendingMessageSizesQueue;

    static void fail(errorCode ec, char const* what);
    void readMore(errorCode ec, int bytesRead, void (T::*func)());
    void packetReceived(int consumeBytes, void (T::*func)());
    void packetReceivedComposite(errorCode ec, int bytesRead, int consumingBytes, void (T::*func)());

    void readMore(errorCode ec, int bytesRead, void (T::*func)(tcp::socket sok));
    void packetReceived(int consumeBytes, void (T::*func)(tcp::socket sok));
    void packetReceivedComposite(errorCode ec, int bytesRead, int consumingBytes, void (T::*func)(tcp::socket sok));

public:
    session(tcp::socket socket, std::shared_ptr<T> state);
    ~session();
    void sendMessage(void (T::*func)());
    void receiveMessage(void (T::*func)());

    void sendMessage(void (T::*func)(tcp::socket sok));
    void receiveMessage(void (T::*func)(tcp::socket sok));
};

//CPP FILE contents
//
// Created by hassan on 10/27/20.
//


#include <iostream>
#include <utility>
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


template <typename T>
void
session<T>::
sendMessage(void (T::*func)()) {
    out << *state;

    sendingMessagesQueue.emplace(sessionStreamBuff.data());
    sendingMessageSizesQueue.emplace(sessionStreamBuff.size());

    sessionStreamBuff.consume(sessionStreamBuff.size());

    std::vector<net::const_buffer> vec;
    vec.emplace_back(reinterpret_cast<char *>(&sendingMessageSizesQueue.front()), sizeof(sendingMessageSizesQueue.front()));
    vec.emplace_back(sendingMessagesQueue.front());
    net::async_write(sock, vec,
                     [self = shared_from_this<T>(), func](
                             errorCode ec, std::size_t bytes) {
                         if(ec){
                             return self->fail(ec, "sendMessage");
                         }
                         self->sendingMessageSizesQueue.pop();
                         self->sendingMessagesQueue.pop();
                         self->nextState.*func();
                     });
}
template <typename T>
void
session<T>::
receiveMessage(void (T::*func)()) {
    //TODO
    //1500 is TCP MTU size. Provide it from somewhere else.
    sock.async_receive(sessionStreamBuff.prepare(1500), [self = shared_from_this<T>(), func](
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
        return fail(ec, "readMore");

    sessionStreamBuff.commit(bytesRead);
    int packetSize = 0;
    in.read(reinterpret_cast<char*>(packetSize), sizeof(packetSize));
    if(packetSize == bytesRead - 4){
        packetReceived(bytesRead, func);
    }
    else if(packetSize > bytesRead-4){
        //Start Composite Asynchronous Operation Before Reporting
        //The Received data.
        int remainingBytes = packetSize - (bytesRead - 4);
        net::async_read(sock, sessionStreamBuff.prepare(remainingBytes),
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
    sessionStreamBuff.consume(consumeBytes);
    state.*func();
}

template <typename T>
void
session<T>::
packetReceivedComposite(errorCode ec, int bytesRead, int consumingBytes, void (T::*func)()) {
    if(ec)
        return fail(ec, "packetReceivedComposite");

    packetReceived(consumingBytes, func);
}

template <typename T>
void
session<T>::
sendMessage(void (T::*func)(tcp::socket sok)) {
    out << *state;

    sendingMessagesQueue.emplace(sessionStreamBuff.data());
    sendingMessageSizesQueue.emplace(sessionStreamBuff.size());

    sessionStreamBuff.consume(sessionStreamBuff.size());

    std::vector<net::const_buffer> vec;
    vec.emplace_back(reinterpret_cast<char *>(&sendingMessageSizesQueue.front()), sizeof(sendingMessageSizesQueue.front()));
    vec.emplace_back(sendingMessagesQueue.front());
    net::async_write(sock, vec,
                     [self = shared_from_this<T>(), func](
                             errorCode ec, std::size_t bytes) {
                         if(ec){
                             return self->fail(ec, "sendMessage");
                         }
                         self->sendingMessageSizesQueue.pop();
                         self->sendingMessagesQueue.pop();
                         self->nextState.*func(std::move(self->sock));
                     });
}
template <typename T>
void
session<T>::
receiveMessage(void (T::*func)(tcp::socket sok)) {
    //TODO
    //1500 is TCP MTU size. Provide it from somewhere else.
    sock.async_receive(sessionStreamBuff.prepare(1500), [self = shared_from_this<T>(), func](
            errorCode ec, std::size_t bytes)
    {
        self->readMore(ec, bytes, func);
    });
}

template <typename T>
void
session<T>::
readMore(errorCode ec, int bytesRead, void (T::*func)(tcp::socket sok)) {
    if(ec)
        return fail(ec, "readMore");

    sessionStreamBuff.commit(bytesRead);
    int packetSize = 0;
    in.read(reinterpret_cast<char*>(packetSize), sizeof(packetSize));
    if(packetSize == bytesRead - 4){
        packetReceived(bytesRead, func);
    }
    else if(packetSize > bytesRead-4){
        //Start Composite Asynchronous Operation Before Reporting
        //The Received data.
        int remainingBytes = packetSize - (bytesRead - 4);
        net::async_read(sock, sessionStreamBuff.prepare(remainingBytes),
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
packetReceived(int consumeBytes, void (T::*func)(tcp::socket sok)) {
//Whole packet Received in One Call
    in >> *state;
    sessionStreamBuff.consume(consumeBytes);
    state.*func(std::move(sock));
}

template <typename T>
void
session<T>::
packetReceivedComposite(errorCode ec, int bytesRead, int consumingBytes, void (T::*func)(tcp::socket sok)) {
    if(ec)
        return fail(ec, "packetReceivedComposite");

    packetReceived(consumingBytes, func);
}

#endif //GETAWAY_SESSION_HPP
