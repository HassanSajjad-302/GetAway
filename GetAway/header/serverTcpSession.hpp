#ifndef GETAWAY_CLIENTTCPSESSION_HPP
#define GETAWAY_CLIENTTCPSESSION_HPP

#include "serverTcpSessionState.hpp"
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
class serverTcpSession : public std::enable_shared_from_this<serverTcpSession>
{
    inline static int count = 0;
    tcp::socket sock;
    std::shared_ptr<serverTcpSessionState> state;
    net::streambuf serverTcpSessionStreamBuff;
    std::istream is{&serverTcpSessionStreamBuff};
    static void fail(errorCode ec, char const* what);
    void onRead(errorCode ec, std::size_t numbOfBytes);

public:
    serverTcpSession(
        tcp::socket socket,
        std::shared_ptr<serverTcpSessionState>  state);
    ~serverTcpSession();
    void run();

};

#endif //GETAWAY_CLIENTTCPSESSION_HPP
