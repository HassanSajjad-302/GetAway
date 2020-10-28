//
// Created by hassan on 10/28/20.
//

#ifndef GETAWAY_SESSIONID_HPP
#define GETAWAY_SESSIONID_HPP

#include <concepts>
#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <boost/asio.hpp>

class ServerLobbyManager;


template <typename T>
concept MultipleSessionManager = requires(T a, int c){
    {c} -> std::same_as<int>;
};

namespace net = boost::asio;
using namespace net::ip;
using errorCode = boost::system::error_code;

template <MultipleSessionManager T>
class sessionID : public std::enable_shared_from_this<T>
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
    sessionID(tcp::socket socket, std::shared_ptr<T> state);
    ~sessionID();
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

template <MultipleSessionManager T>
sessionID<T>::
sessionID(
        tcp::socket socket,
        std::shared_ptr<T>  state)
        : sock(std::move(socket))
        , state(std::move(state))
{
    this->id = state->join(*this);
}

template<MultipleSessionManager T>
sessionID<T>::~sessionID(){
    this->leave(id);
}

// Report a failure
template <MultipleSessionManager T>
void
sessionID<T>::
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
template <MultipleSessionManager T>
void
sessionID<T>::
sendMessage(void (T::*func)(int id)) {
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
                         self->nextState.*func(self->id);
                     });
}
template <MultipleSessionManager T>
void
sessionID<T>::
receiveMessage(void (T::*func)(int id)) {
    //TODO
    //1500 is TCP MTU size. Provide it from somewhere else.
    sock.async_receive(sessionStreamBuff.prepare(1500), [self = shared_from_this<T>(), func](
            errorCode ec, std::size_t bytes)
    {
        self->readMore(ec, bytes, func);
    });
}

template <MultipleSessionManager T>
void
sessionID<T>::
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

template <MultipleSessionManager T>
void
sessionID<T>::
packetReceived(int consumeBytes, void (T::*func)(int id)) {
//Whole packet Received in One Call
    in >> *state;
    sessionStreamBuff.consume(consumeBytes);
    state.*func(state->id);
}

template <MultipleSessionManager T>
void
sessionID<T>::
packetReceivedComposite(errorCode ec, int bytesRead, int consumingBytes, void (T::*func)(int id)) {
    if(ec)
        return fail(ec, "packetReceivedComposite");

    packetReceived(consumingBytes, func);
}


#endif //GETAWAY_SESSIONID_HPP
