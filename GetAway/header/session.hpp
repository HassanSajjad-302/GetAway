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
    boost::asio::streambuf lobbySessionStreamBuff;
    std::ostream out{&lobbySessionStreamBuff};
    std::istream in{&lobbySessionStreamBuff};
    std::queue<net::const_buffer> sendingMessagesQueue;
    std::queue<int> sendingMessageSizesQueue;

    static void fail(errorCode ec, char const* what);
    void readMore(errorCode ec, int bytesRead, void (T::*func)());
    void packetReceived(int consumeBytes, void (T::*func)());
    void packetReceivedComposite(errorCode ec, int bytesRead, int consumingBytes, void (T::*func)());

public:
    session(tcp::socket socket, std::shared_ptr<T> state);
    ~session();
    void sendMessage(void (T::*func)());
    void receiveMessage(void (T::*func)());
};


#endif //GETAWAY_SESSION_HPP
