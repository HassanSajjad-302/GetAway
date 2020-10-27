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


template<typename T>
clientTcpSession<T>::clientTcpSession(tcp::socket socket, std::shared_ptr<T> state) :
sock(std::move(socket)), state(std::move(state))
{

}

template <typename T>
clientTcpSession<T>::~clientTcpSession()= default;

template <typename T>
void
clientTcpSession<T>::
run()
{
    out << state->getClassWriteSize();
    out << state;
    net::async_write(sock, tcpSessionStreamBuff.data(),
                    [self = shared_from_this<T>()](errorCode ec, std::size_t bytes_sent){
        self->tcpSessionStreamBuff.consume(bytes_sent);
        std::make_shared<clientLobbySession>
    });
}

// Report a failure
template <typename T>
void
clientTcpSession<T>::
fail(errorCode ec, char const* what)
{
    // Don't report on canceled operations
    if(ec == net::error::operation_aborted)
        return;

    std::cerr << what << ": " << ec.message() << "\n";
}


