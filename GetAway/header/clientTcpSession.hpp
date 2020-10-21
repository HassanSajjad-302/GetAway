#ifndef CPPCON2018_TCP_SESSION_HPP
#define CPPCON2018_TCP_SESSION_HPP

#include "clientState.hpp"
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <boost/asio.hpp>


namespace net = boost::asio;
using namespace net::ip;
using errorCode = boost::system::error_code;


/** Represents an established TCP connection
*/
class clientTcpSession : public std::enable_shared_from_this<clientTcpSession>
{
    inline static int count = 0;
    tcp::socket socket_;
    std::shared_ptr<clientState> state;
    boost::asio::streambuf tcpSessionStreamBuff;
    std::string password = "password";
    const int nameLength = 60; //Maximum chars for name
    unsigned int maxBufferSize = nameLength + password.size() + 2; // 2 is added to compensate for 2 \n
    static void fail(errorCode ec, char const* what);
    void onRead(errorCode ec, std::size_t numbOfBytes);

public:
    clientTcpSession(
        tcp::socket socket,
        std::shared_ptr<clientState>  state);
    ~clientTcpSession();
    void run();

};

#endif
