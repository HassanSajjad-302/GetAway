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
#include "Log_Macro.hpp"

namespace net = boost::asio;
using namespace net::ip;
using errorCode = boost::system::error_code;

template <typename T, bool ID = false>
class session : public std::enable_shared_from_this<session<T, false>>
{
    std::shared_ptr<T> managerPtr;
    boost::asio::streambuf sessionStreamBuff;
    std::ostream out{&sessionStreamBuff};
    std::istream in{&sessionStreamBuff};
    std::queue<net::const_buffer> sendingMessagesQueue;
    std::queue<int> sendingMessageSizesQueue;

    static void fail(errorCode ec, char const* what);
    void readMore(errorCode ec, int bytesRead);
    void packetReceived(int consumeBytes);
    void packetReceivedComposite(errorCode ec, int bytesRead, int consumingBytes);



    //friend T;
public:
    tcp::socket sock;
    void sendMessage(void (T::*func)());
    void receiveMessage();
    void registerSessionToManager();
    session(tcp::socket socket, std::shared_ptr<T> state);
    ~session();

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
        , managerPtr(std::move(state))
{

}

template<typename T, bool ID>
void session<T, ID>::registerSessionToManager() {
    managerPtr->join(this->shared_from_this());
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
    out << *managerPtr;

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
                         (*(self->managerPtr).*func)();
                     });
}

template <typename T, bool ID>
void
session<T, ID>::
receiveMessage() {
    //TODO
    //1500 is TCP MTU size. Provide it from somewhere else.
    sock.async_receive(sessionStreamBuff.prepare(1500), [self = this->shared_from_this()](
            errorCode ec, std::size_t bytes)
    {
        self->readMore(ec, bytes);
    });
}

template <typename T, bool ID>
void
session<T, ID>::
readMore(errorCode ec, int bytesRead) {
    if(ec)
        return fail(ec, "readMore");

    sessionStreamBuff.commit(bytesRead);
    int packetSize = 0;
    in.read(reinterpret_cast<char*>(&packetSize), sizeof(packetSize));
    if(packetSize == bytesRead - 4){
        packetReceived(bytesRead);
    }
    else if(packetSize > bytesRead-4){
        //Start Composite Asynchronous Operation Before Reporting
        //The Received data.
        int remainingBytes = packetSize - (bytesRead - 4);
        net::async_read(sock, sessionStreamBuff.prepare(remainingBytes),
                        [self = this->shared_from_this(), bytesRead, remainingBytes](
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

template <typename T, bool ID>
void
session<T, ID>::
packetReceived(int consumeBytes) {
//Whole packet Received in One Call
    managerPtr->receivedPacketSize = consumeBytes - 4;
    in >> *managerPtr;
    sessionStreamBuff.consume(consumeBytes);
}

template <typename T, bool ID>
void
session<T, ID>::
packetReceivedComposite(errorCode ec, int bytesRead, int consumingBytes) {
    if(ec)
        return fail(ec, "packetReceivedComposite");

    packetReceived(consumingBytes);
}

template <typename T>
class session<T, true> : public std::enable_shared_from_this<session<T,true>>
{
    int id= 0;
    std::shared_ptr<T> managerPtr;
    boost::asio::streambuf sessionStreamBuff;
    std::ostream out{&sessionStreamBuff};
    std::istream in{&sessionStreamBuff};
    std::queue<net::const_buffer> sendingMessagesQueue;
    std::queue<int> sendingMessageSizesQueue;

    static void fail(errorCode ec, char const* what);
    void readMore(errorCode ec, int bytesRead);
    void packetReceived(int consumeBytes);
    void packetReceivedComposite(errorCode ec, int bytesRead, int consumingBytes);

    //friend T;
public:
    tcp::socket sock;
    void sendMessage(void (T::*func)(int id));
    void receiveMessage();
    void registerSessionToManager();

    session(tcp::socket socket, std::shared_ptr<T> state);
    ~session();

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
        , managerPtr(std::move(state))
{
#ifdef LOG
    spdlog::info("Session Constructor Called");
#endif
}

template<typename T>
void session<T, true>::registerSessionToManager() {
#ifdef LOG
    spdlog::info("{}\t{}\t{}",__FILE__,__FUNCTION__ ,__LINE__);
#endif
    this->id = managerPtr->join(std::enable_shared_from_this<session<T, true>>::shared_from_this());
#ifdef LOG
    spdlog::info("{}\t{}\t{}",__FILE__,__FUNCTION__ ,__LINE__);
#endif
}

template<typename T>
session<T, true>::~session(){
    spdlog::info("Session Destructor Called");
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
    out << *managerPtr;

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
                         self->managerPtr->excitedSessionId = self->id;
                         (*(self->managerPtr).*func)(self->id);
                     });
}
template <typename T>
void
session<T, true>::
receiveMessage() {
    //TODO
    //1500 is TCP MTU size. Provide it from somewhere else.
    sock.async_receive(sessionStreamBuff.prepare(1500), [self = this->shared_from_this()](
            errorCode ec, std::size_t bytes)
    {
        self->readMore(ec, bytes);
    });
}

template <typename T>
void
session<T, true>::
readMore(errorCode ec, int bytesRead) {
    if(ec)
        return fail(ec, "readMore");

    sessionStreamBuff.commit(bytesRead);
    int packetSize = 0;
    in.read(reinterpret_cast<char*>(&packetSize), sizeof(packetSize));
    if(packetSize == bytesRead - 4){
        packetReceived(bytesRead);
    }
    else if(packetSize > bytesRead-4){
        //Start Composite Asynchronous Operation Before Reporting
        //The Received data.
        int remainingBytes = packetSize - (bytesRead - 4);
        net::async_read(sock, sessionStreamBuff.prepare(remainingBytes),
                        [self = this->shared_from_this(), bytesRead, remainingBytes](
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

template <typename T>
void
session<T, true>::
packetReceived(int consumeBytes) {
//Whole packet Received in One Call
    managerPtr->receivedPacketSize = consumeBytes - 4;
    managerPtr->excitedSessionId= id;
    in >> *managerPtr;
    sessionStreamBuff.consume(consumeBytes);
}

template <typename T>
void
session<T, true>::
packetReceivedComposite(errorCode ec, int bytesRead, int consumingBytes) {
    if(ec)
        return fail(ec, "packetReceivedComposite");

    packetReceived(consumingBytes);
}

#endif //GETAWAY_SESSION_HPP
