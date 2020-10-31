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

template <typename T, bool ID = false>
class session : public std::enable_shared_from_this<session<T, false>>
{
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

public:
    tcp::socket sock;
    session(tcp::socket socket, std::shared_ptr<T> state);
    ~session();
    void sendMessage(void (T::*func)());
    void receiveMessage(void (T::*func)());
    void registerSessionToManager();

    int receivedPacketSize;
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

template <typename T, bool ID>
session<T, ID>::
session(
        tcp::socket socket,
        std::shared_ptr<T>  state)
        : sock(std::move(socket))
        , state(std::move(state))
{

}
template<typename T, bool ID>
void session<T, ID>::registerSessionToManager() {
    state->join(this->shared_from_this());
}
template<typename T, bool ID>
session<T, ID>::~session() = default;

// Report a failure
template <typename T, bool ID>
void
session<T, ID>::
fail(errorCode ec, char const* what)
{
    // Don't report on canceled operations
    if(ec == net::error::operation_aborted)
        return;

    std::cerr << what << ": " << ec.message() << "\n";
}


template <typename T, bool ID>
void
session<T, ID>::
sendMessage(void (T::*func)()) {
    std::cout<<"sendMessage called"<<std::endl;
    out << *state;

    sendingMessagesQueue.emplace(sessionStreamBuff.data());
    sendingMessageSizesQueue.emplace(sessionStreamBuff.size());

    sessionStreamBuff.consume(sessionStreamBuff.size());

    std::vector<net::const_buffer> vec;
    vec.emplace_back(reinterpret_cast<char *>(&sendingMessageSizesQueue.front()), sizeof(sendingMessageSizesQueue.front()));
    vec.emplace_back(sendingMessagesQueue.front());
    net::async_write(sock, vec,
                     [self = this->shared_from_this(), func](
                             errorCode ec, std::size_t bytes) {
                         if(ec){
                             return self->fail(ec, "sendMessage");
                         }
                         self->sendingMessageSizesQueue.pop();
                         self->sendingMessagesQueue.pop();
                         (*(self->state).*func)();
                     });
}

template <typename T, bool ID>
void
session<T, ID>::
receiveMessage(void (T::*func)()) {
    //TODO
    //1500 is TCP MTU size. Provide it from somewhere else.
    sock.async_receive(sessionStreamBuff.prepare(1500), [self = this->shared_from_this(), func](
            errorCode ec, std::size_t bytes)
    {
        self->readMore(ec, bytes, func);
    });
}

template <typename T, bool ID>
void
session<T, ID>::
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
                        [self = this->shared_from_this(), bytesRead, remainingBytes, func](
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

template <typename T, bool ID>
void
session<T, ID>::
packetReceived(int consumeBytes, void (T::*func)()) {
//Whole packet Received in One Call
    receivedPacketSize = consumeBytes -4;
    in >> *state;
    sessionStreamBuff.consume(consumeBytes);
    state.*func();
}

template <typename T, bool ID>
void
session<T, ID>::
packetReceivedComposite(errorCode ec, int bytesRead, int consumingBytes, void (T::*func)()) {
    if(ec)
        return fail(ec, "packetReceivedComposite");

    packetReceived(consumingBytes, func);
}

template <typename T>
class session<T, true> : public std::enable_shared_from_this<session<T,true>>
{
    int id= 0;
    tcp::socket sock;
    std::shared_ptr<T> state;
    boost::asio::streambuf sessionStreamBuff;
    std::ostream out{&sessionStreamBuff};
    std::istream in{&sessionStreamBuff};
    std::queue<net::const_buffer> sendingMessagesQueue;
    std::queue<int> sendingMessageSizesQueue;

    static void fail(errorCode ec, char const* what);
    void readMore(errorCode ec, int bytesRead, void (T::*func)(int id));
    void packetReceived(int consumeBytes, void (T::*func)(int id));
    void packetReceivedComposite(errorCode ec, int bytesRead, int consumingBytes, void (T::*func)(int id));

public:
    int getId();
    session(tcp::socket socket, std::shared_ptr<T> state);
    ~session();
    void sendMessage(void (T::*func)(int id));
    void receiveMessage(void (T::*func)(int id));
};


//CPP FILE COTENTS
//
// Created by hassan on 10/28/20.
//

//This is same session
//class but with join, leave and id reporting capabilities.

#include <iostream>
#include <boost/asio.hpp>
#include <utility>
#include "Log_Macro.hpp"

namespace net = boost::asio;
using namespace net::ip;
using errorCode = boost::system::error_code;
//------------------------------------------------------------------------------

template <typename T>
session<T, true>::
session(
        tcp::socket socket,
        std::shared_ptr<T>  state)
        : sock(std::move(socket))
        , state(std::move(state))
{
    this->id = state->join(this->shared_from_this());
}

template<typename T>
session<T, true>::~session(){
    this->leave(id);
}

// Report a failure
template <typename T>
void
session<T, true>::
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
session<T, true>::
sendMessage(void (T::*func)(int id)) {
    out << *state;

    sendingMessagesQueue.emplace(sessionStreamBuff.data());
    sendingMessageSizesQueue.emplace(sessionStreamBuff.size());

    sessionStreamBuff.consume(sessionStreamBuff.size());

    std::vector<net::const_buffer> vec;
    vec.emplace_back(reinterpret_cast<char *>(&sendingMessageSizesQueue.front()), sizeof(sendingMessageSizesQueue.front()));
    vec.emplace_back(sendingMessagesQueue.front());
    net::async_write(sock, vec,
                     [self = this->shared_from_this(), func](
                             errorCode ec, std::size_t bytes) {
                         if(ec){
                             return self->fail(ec, "sendMessage");
                         }
                         self->sendingMessageSizesQueue.pop();
                         self->sendingMessagesQueue.pop();
                         self->nextManager.*func(self->id);
                     });
}
template <typename T>
void
session<T, true>::
receiveMessage(void (T::*func)(int id)) {
    //TODO
    //1500 is TCP MTU size. Provide it from somewhere else.
    sock.async_receive(sessionStreamBuff.prepare(1500), [self = this->shared_from_this(), func](
            errorCode ec, std::size_t bytes)
    {
        self->readMore(ec, bytes, func);
    });
}

template <typename T>
void
session<T, true>::
readMore(errorCode ec, int bytesRead, void (T::*func)(int id)) {
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
                        [self = this->shared_from_this(), bytesRead, remainingBytes, func](
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
session<T, true>::
packetReceived(int consumeBytes, void (T::*func)(int id)) {
//Whole packet Received in One Call
    in >> *state;
    sessionStreamBuff.consume(consumeBytes);
    state.*func(state->id);
}

template <typename T>
void
session<T, true>::
packetReceivedComposite(errorCode ec, int bytesRead, int consumingBytes, void (T::*func)(int id)) {
    if(ec)
        return fail(ec, "packetReceivedComposite");

    packetReceived(consumingBytes, func);
}

#endif //GETAWAY_SESSION_HPP
