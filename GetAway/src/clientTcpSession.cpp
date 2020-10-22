#include "clientTcpSession.hpp"
#include "clientLobbySession.hpp"
#include <iostream>
#include <boost/asio.hpp>
#include <regex>
#include <utility>
#include "Log_Macro.hpp"
#include "spdlog/spdlog.h"
#include "messageProcessing.hpp"
namespace net = boost::asio;
using namespace net::ip;
using errorCode = boost::system::error_code;
//------------------------------------------------------------------------------

clientTcpSession::
clientTcpSession(
    tcp::socket socket,
    std::shared_ptr<clientTcpSessionState>  state)
    : sock(std::move(socket))
    , state(std::move(state))
{
}
clientTcpSession::~clientTcpSession()= default;

void func(errorCode& ec, std::size_t bytes){

}

void
clientTcpSession::
run()
{
    int size = tcpSessionStreamBuff.size();
    out << state;
    size -= tcpSessionStreamBuff.size();
    sock.async_send(tcpSessionStreamBuff.data(), [](errorCode ec, std::size_t bytes_sent){
        std::make_shared<clientLobbySession>()
    });
    sock.write_some(tcpSessionStreamBuff.data());
}

// Report a failure
void
clientTcpSession::
fail(errorCode ec, char const* what)
{
    // Don't report on canceled operations
    if(ec == net::error::operation_aborted)
        return;

    std::cerr << what << ": " << ec.message() << "\n";
}

