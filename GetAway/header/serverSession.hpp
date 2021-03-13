
#ifndef GETAWAY_SERVERSESSION_HPP
#define GETAWAY_SERVERSESSION_HPP

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
#include "constants.hpp"

using namespace asio::ip;
using errorCode = asio::error_code;

template <typename T>
class serverSession : public std::enable_shared_from_this<serverSession<T>>
{
    int id= 0;
    T& manager;
    asio::streambuf sessionStreamBuffInput;
    std::istream in{&sessionStreamBuffInput};
    asio::streambuf sessionStreamBuffOutput;
public:
    std::ostream out{&sessionStreamBuffOutput};

private:
    std::queue<std::vector<asio::const_buffer>> sendingMessagesQueue;

    void fail(errorCode ec, char const* what);
    void readMore(errorCode ec, int bytes);
    bool allPacketsReceived = true;

    //friend T;
public:
    tcp::socket sock;
    void sendMessage(void (*func)(int id));
    void sendMessage();
    void receiveMessage();

    serverSession(tcp::socket socket, T& manager_, int id_);
};

template <typename T>
serverSession<T>::
serverSession(
        tcp::socket socket,
        T&  manager_,
        int id_)
        : sock(std::move(socket))
        , manager(manager_)
        , id(id_)
{
}

// Report a failure
template <typename T>
void
serverSession<T>::
fail(errorCode ec, char const* what)
{
    if(ec == asio::error::operation_aborted)
        return;
    resourceStrings::print(std::string(what) + ": " + ec.message() + "\r\n");
    manager.leave(id);
}

template <typename T>
void
serverSession<T>::
sendMessage(void (*func)(int id)) {

    int msSize = sessionStreamBuffOutput.data().size();

    std::vector<asio::const_buffer> vec;
    vec.emplace_back(reinterpret_cast<char *>(&msSize), sizeof(msSize));
    vec.emplace_back(sessionStreamBuffOutput.data());

    sendingMessagesQueue.emplace(std::move(vec));
    sessionStreamBuffOutput.consume(sessionStreamBuffOutput.size());

    asio::async_write(sock, sendingMessagesQueue.front(),
                     [self = this->shared_from_this(), func, id = id](
                             errorCode ec, std::size_t bytes) {
                         if(ec){
                             return self->fail(ec, "sendMessage");
                         }
                         func(id);
                     });
    sendingMessagesQueue.pop();
}

template <typename T>
void
serverSession<T>::
sendMessage() {

    int msSize = sessionStreamBuffOutput.data().size();

    std::vector<asio::const_buffer> vec;
    vec.emplace_back(reinterpret_cast<char *>(&msSize), sizeof(msSize));
    vec.emplace_back(sessionStreamBuffOutput.data());

    sendingMessagesQueue.emplace(std::move(vec));
    sessionStreamBuffOutput.consume(sessionStreamBuffOutput.size());

    asio::async_write(sock, sendingMessagesQueue.front(),
                      [self = this](
                              errorCode ec, std::size_t bytes) {
                          if(ec){
                              return self->fail(ec, "sendMessage");
                          }
                      });
    sendingMessagesQueue.pop();
}

template <typename T>
void
serverSession<T>::
receiveMessage() {
    if(allPacketsReceived){
        sock.async_receive(sessionStreamBuffInput.prepare(constants::TCP_PACKET_MTU), [self = this](
                errorCode ec, std::size_t bytes)
        {
            self->readMore(ec, bytes);
        });
    }

}

template <typename T>
void
serverSession<T>::
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
            manager.packetReceivedFromNetwork(in, packetSize, id);

            sessionStreamBuffInput.consume(firstRead);
            //sessionStreamBuffInput.consume(SIZE_MAX);
            break;
        } else if (packetSize > waitingForServiceBody) {
            //if it has come here then it means that consumeSize is firstRead plus 1500 which is standard read.
            int remainingPacket = packetSize - waitingForServiceBody;
            asio::async_read(sock, sessionStreamBuffInput.prepare(remainingPacket),
                            [self = this, waitingForServiceBody, firstRead, remainingPacket](
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
                                        self->in, secondRead + waitingForServiceBody, self->id);
                                self->sessionStreamBuffInput.consume(firstRead + secondRead);
                            });
            break;
        }
        else {
            allPacketsReceived = false;
            manager.packetReceivedFromNetwork(in, packetSize, id);
            waitingForService -= (packetSize + 4);
        }
    }
}

#endif //GETAWAY_SERVERSESSION_HPP
