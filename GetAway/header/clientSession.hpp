
#ifndef GETAWAY_CLIENTSESSION_H
#define GETAWAY_CLIENTSESSION_H

#include <memory>
#include <string>
#include <vector>
#include <queue>
#include "asio/error_code.hpp"
#include "asio/streambuf.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/write.hpp"
#include "asio/read.hpp"
#include "resourceStrings.hpp"
#include "constants.h"

using namespace asio::ip;
using errorCode = asio::error_code;

template <typename T, typename ...U>
class clientSession : public std::enable_shared_from_this<clientSession<T, U...>>
{
    T manager;
    asio::streambuf sessionStreamBuffInput;
    std::istream in{&sessionStreamBuffInput};
    asio::streambuf sessionStreamBuffOutput;
public:
    std::ostream out{&sessionStreamBuffOutput};
private:
    std::queue<std::vector<asio::const_buffer>> sendingMessagesQueue;
    bool allPacketsReceived = true;

    static void fail(errorCode ec, char const* what);
    void readMore(errorCode ec, int firstRead);

public:
    tcp::socket sock;
    void sendMessage(void (*func)());
    void sendMessage();
    void receiveMessage();
    void run();

    explicit clientSession(tcp::socket socket, U... stateConstructorArgs);
};

template<typename T, typename... U>
clientSession<T, U...>::clientSession(tcp::socket socket, U... stateConstructorArgs) :
sock(std::move(socket)), manager(*this, stateConstructorArgs...){

}

// Report a failure
template <typename T, typename ...U>
void
clientSession<T, U...>::
fail(errorCode ec, char const* what)
{

    resourceStrings::print(std::string(what) + ": " + ec.message() + "\r\n");

    // Don't report on canceled operations
    if(ec == asio::error::operation_aborted)
        return;
}


template <typename T, typename ...U>
void
clientSession<T, U...>::
sendMessage(void (*func)()) {

    int msSize = sessionStreamBuffOutput.data().size();

    std::vector<asio::const_buffer> vec;
    vec.emplace_back(reinterpret_cast<char *>(&msSize), sizeof(msSize));
    vec.emplace_back(sessionStreamBuffOutput.data());

    sendingMessagesQueue.emplace(std::move(vec));
    sessionStreamBuffOutput.consume(sessionStreamBuffOutput.size());

    asio::async_write(sock, sendingMessagesQueue.front(),
                      [self = this->shared_from_this(), func](
                              errorCode ec, std::size_t bytes) {

                          if(ec){
                              return self->fail(ec, "sendMessage");
                          }
                          func();
                      });
    sendingMessagesQueue.pop();
}

template <typename T, typename ...U>
void
clientSession<T, U...>::
sendMessage(){

    int msSize = sessionStreamBuffOutput.data().size();

    std::vector<asio::const_buffer> vec;
    vec.emplace_back(reinterpret_cast<char *>(&msSize), sizeof(msSize));
    vec.emplace_back(sessionStreamBuffOutput.data());

    sendingMessagesQueue.emplace(std::move(vec));
    sessionStreamBuffOutput.consume(sessionStreamBuffOutput.size());

    asio::async_write(sock, sendingMessagesQueue.front(),
                      [self = this->shared_from_this()](
                              errorCode ec, std::size_t bytes) {
                          if(ec){
                              return self->fail(ec, "sendMessage");
                          }
                      });
    sendingMessagesQueue.pop();
}

template <typename T, typename ...U>
void
clientSession<T, U...>::
receiveMessage() {
    if(allPacketsReceived){
        sock.async_receive(sessionStreamBuffInput.prepare(constants::TCP_PACKET_MTU), [self = this->shared_from_this()](
                errorCode ec, std::size_t bytes)
        {
            self->readMore(ec, bytes);
        });
    }

}

template <typename T, typename ...U>
void
clientSession<T, U...>::
readMore(errorCode ec, int firstRead) {
    if(ec){
        manager.readMoreFailInClientSession(ec);
        return fail(ec, "readMore");
    }

    sessionStreamBuffInput.commit(firstRead);
    int packetSize = 0;
    int waitingForService = firstRead;
    while(true) {
        in.read(reinterpret_cast<char *>(&packetSize), sizeof(packetSize));

        int waitingForServiceBody = waitingForService - 4;
        if (packetSize == waitingForServiceBody) {
            allPacketsReceived = true;
            manager.packetReceivedFromNetwork(in, packetSize);

            sessionStreamBuffInput.consume(firstRead);
            //sessionStreamBuffInput.consume(SIZE_MAX);
            break;
        } else if (packetSize > waitingForServiceBody) {
            //if it has come here then it means that consumeSize is firstRead plus 1500 which is standard read.
            int remainingPacket = packetSize - waitingForServiceBody;
            asio::async_read(sock, sessionStreamBuffInput.prepare(remainingPacket),
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
                                 self->manager.packetReceivedFromNetwork(
                                         self->in, secondRead + waitingForServiceBody);
                                 self->sessionStreamBuffInput.consume(firstRead + secondRead);
                             });
            break;
        }
        else {
            allPacketsReceived = false;
            manager.packetReceivedFromNetwork(in, packetSize);
            waitingForService -= (packetSize + 4);
        }
    }
}

template<typename T, typename... U>
void clientSession<T, U...>::run() {
    manager.run();
}

template<typename T> clientSession(T, asio::io_context, std::string) -> clientSession<T, asio::io_context&, std::string>;
#endif //GETAWAY_CLIENTSESSION_H
