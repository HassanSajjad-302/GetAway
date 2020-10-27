#ifndef GETAWAY_CLIENTTCPSESSION_HPP
#define GETAWAY_CLIENTTCPSESSION_HPP

#include "clientTcpSessionState.hpp"
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

template <typename T>
class clientTcpSession : public std::enable_shared_from_this<clientTcpSession<T>>
{
    tcp::socket sock;
    std::shared_ptr<T> state;
    boost::asio::streambuf tcpSessionStreamBuff;
    std::ostream out{&tcpSessionStreamBuff};
    static void fail(errorCode ec, char const* what);

public:
    clientTcpSession(
        tcp::socket socket,
        std::shared_ptr<T>  state);
    ~clientTcpSession();
    void run();
};

#endif //GETAWAY_CLIENTTCPSESSION_HPP
