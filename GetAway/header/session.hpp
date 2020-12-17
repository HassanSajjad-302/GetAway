//
// Created by hassan on 10/27/20.
//

#ifndef GETAWAY_SESSION_HPP
#define GETAWAY_SESSION_HPP

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <boost/asio.hpp>

#define LOG
namespace net = boost::asio;
using namespace net::ip;
using errorCode = boost::system::error_code;

template <typename T, bool ID = false>
class session : public std::enable_shared_from_this<session<T, false>>
{
    std::shared_ptr<T> managerPtr;
    boost::asio::streambuf sessionStreamBuffInput;
    std::istream in{&sessionStreamBuffInput};
    net::streambuf sessionStreamBuffOutput;
public:
    std::ostream out{&sessionStreamBuffOutput};
private:
    std::queue<std::vector<net::const_buffer>> sendingMessagesQueue;
    bool allPacketsReceived = true;

    static void fail(errorCode ec, char const* what);
    void readMore(errorCode ec, int bytes);

public:
    tcp::socket sock;
    void sendMessage(void (T::*func)());
    void receiveMessage();
    void registerSessionToManager();
    session(tcp::socket socket, std::shared_ptr<T> state);
    ~session();

};

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
session<T, ID>::~session(){
}

template<typename T, bool ID>
void session<T, ID>::registerSessionToManager() {
    managerPtr->join(this->shared_from_this());
}

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

    int msSize = sessionStreamBuffOutput.data().size();

    std::vector<net::const_buffer> vec;
    vec.emplace_back(reinterpret_cast<char *>(&msSize), sizeof(msSize));
    vec.emplace_back(sessionStreamBuffOutput.data());

    sendingMessagesQueue.emplace(std::move(vec));
    sessionStreamBuffOutput.consume(sessionStreamBuffOutput.size());

    net::async_write(sock, sendingMessagesQueue.front(),
                     [self = this->shared_from_this(), func](
                             errorCode ec, std::size_t bytes) {

                         if(ec){
                             return self->fail(ec, "sendMessage");
                         }
                         (*(self->managerPtr).*func)();
                     });
    sendingMessagesQueue.pop();
}

template <typename T, bool ID>
void
session<T, ID>::
receiveMessage() {
    if(allPacketsReceived){
        //TODO
        //1500 is TCP MTU size. Provide it from somewhere else.
        sock.async_receive(sessionStreamBuffInput.prepare(1500), [self = this->shared_from_this()](
                errorCode ec, std::size_t bytes)
        {
            self->readMore(ec, bytes);
        });
    }

}

template <typename T, bool ID>
void
session<T, ID>::
readMore(errorCode ec, int firstRead) {
    if(ec)
        return fail(ec, "readMore");

    sessionStreamBuffInput.commit(firstRead);
    int packetSize = 0;
    int waitingForService = firstRead;
    while(true) {
        in.read(reinterpret_cast<char *>(&packetSize), sizeof(packetSize));

        int waitingForServiceBody = waitingForService - 4;
        if (packetSize == waitingForServiceBody) {
            allPacketsReceived = true;
            managerPtr->packetReceivedFromNetwork(in, packetSize);

            sessionStreamBuffInput.consume(firstRead);
            //sessionStreamBuffInput.consume(SIZE_MAX);
            break;
        } else if (packetSize > waitingForServiceBody) {
            //if it has come here then it means that consumeSize is firstRead plus 1500 which is standard read.
            int remainingPacket = packetSize - waitingForServiceBody;
            net::async_read(sock, sessionStreamBuffInput.prepare(remainingPacket),
                            [self = this->shared_from_this(), waitingForServiceBody, firstRead, remainingPacket](
                                    errorCode ec, std::size_t secondRead) {
                                if (ec)
                                    self->fail(ec, "error in lambda readMore");
                                if(secondRead != remainingPacket){
                                    self->fail(ec, "error in lambda readmore. handler called with not the required "
                                                   "bytes Read");
                                }
                                self->sessionStreamBuffInput.commit(remainingPacket);
                                self->allPacketsReceived = true;
                                self->managerPtr->packetReceivedFromNetwork(
                                        self->in, secondRead + waitingForServiceBody);
                                self->sessionStreamBuffInput.consume(firstRead + secondRead);
                            });
            break;
        }
        else {
            allPacketsReceived = false;
            managerPtr->packetReceivedFromNetwork(in, packetSize);
            waitingForService -= (packetSize + 4);
        }
    }
}

template <typename T>
class session<T, true> : public std::enable_shared_from_this<session<T,true>>
{
    int id= 0;
    std::shared_ptr<T> managerPtr;
    boost::asio::streambuf sessionStreamBuffInput;
    std::istream in{&sessionStreamBuffInput};
    boost::asio::streambuf sessionStreamBuffOutput;
public:
    std::ostream out{&sessionStreamBuffOutput};

private:
    std::queue<std::vector<net::const_buffer>> sendingMessagesQueue;

    void fail(errorCode ec, char const* what);
    void readMore(errorCode ec, int bytes);
    bool allPacketsReceived = true;

    //friend T;
public:
    tcp::socket sock;
    void sendMessage(void (T::*func)(int id));
    void receiveMessage();
    void registerSessionToManager();

    session(tcp::socket socket, std::shared_ptr<T> state);
    ~session();

};

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
}

template<typename T>
void session<T, true>::registerSessionToManager() {
    this->id = managerPtr->join(std::enable_shared_from_this<session<T, true>>::shared_from_this());
}

template<typename T>
session<T, true>::~session(){
}

// Report a failure
template <typename T>
void
session<T, true>::
fail(errorCode ec, char const* what)
{
    //TODO
    //Currently On Operation Aborted I call the leave of manager to unregister
    //the session but I believe I should call the leave on any sort of error
    //Currently Not Sure On That Because I don't know meaning of boost system
    //error codes
    // Don't report on canceled operations

    std::cerr << what << ": " << ec.message() << "\n";
    managerPtr->leave(id);
}

template <typename T>
void
session<T, true>::
sendMessage(void (T::*func)(int id)) {

    int msSize = sessionStreamBuffOutput.data().size();

    std::vector<net::const_buffer> vec;
    vec.emplace_back(reinterpret_cast<char *>(&msSize), sizeof(msSize));
    vec.emplace_back(sessionStreamBuffOutput.data());

    sendingMessagesQueue.emplace(std::move(vec));
    sessionStreamBuffOutput.consume(sessionStreamBuffOutput.size());

    net::async_write(sock, sendingMessagesQueue.front(),
                     [self = this->shared_from_this(), func](
                             errorCode ec, std::size_t bytes) {
                         if(ec){
                             return self->fail(ec, "sendMessage");
                         }
                     });
    sendingMessagesQueue.pop();
}
template <typename T>
void
session<T, true>::
receiveMessage() {
    if(allPacketsReceived){
        //TODO
        //1500 is TCP MTU size. Provide it from somewhere else.
        sock.async_receive(sessionStreamBuffInput.prepare(40), [self = this->shared_from_this()](
                errorCode ec, std::size_t bytes)
        {
            self->readMore(ec, bytes);
        });
    }

}

template <typename T>
void
session<T, true>::
readMore(errorCode ec, int firstRead) {
    if(ec)
        return fail(ec, "readMore");

    sessionStreamBuffInput.commit(firstRead);
    int packetSize = 0;
    int waitingForService = firstRead;
    while(true) {
        in.read(reinterpret_cast<char *>(&packetSize), sizeof(packetSize));

        int waitingForServiceBody = waitingForService - 4;
        if (packetSize == waitingForServiceBody) {
            allPacketsReceived = true;
            managerPtr->packetReceivedFromNetwork(in, packetSize, id);

            sessionStreamBuffInput.consume(firstRead);
            //sessionStreamBuffInput.consume(SIZE_MAX);
            break;
        } else if (packetSize > waitingForServiceBody) {
            //if it has come here then it means that consumeSize is firstRead plus 1500 which is standard read.
            int remainingPacket = packetSize - waitingForServiceBody;
            net::async_read(sock, sessionStreamBuffInput.prepare(remainingPacket),
                            [self = this->shared_from_this(), waitingForServiceBody, firstRead, remainingPacket](
                                    errorCode ec, std::size_t secondRead) {
                                if (ec)
                                    self->fail(ec, "error in lambda readMore");
                                if(secondRead != remainingPacket){
                                    self->fail(ec, "error in lambda readmore. handler called with not the required "
                                                   "bytes Read");
                                }
                                self->sessionStreamBuffInput.commit(remainingPacket);
                                self->allPacketsReceived = true;
                                self->managerPtr->packetReceivedFromNetwork(
                                        self->in, secondRead + waitingForServiceBody, self->id);
                                self->sessionStreamBuffInput.consume(firstRead + secondRead);
                            });
            break;
        }
        else {
            allPacketsReceived = false;
            managerPtr->packetReceivedFromNetwork(in, packetSize, id);
            waitingForService -= (packetSize + 4);
        }
    }
}

#endif //GETAWAY_SESSION_HPP
