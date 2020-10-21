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
    std::shared_ptr<clientState>  state)
    : socket_(std::move(socket))
    , state(std::move(state))
{
}
clientTcpSession::~clientTcpSession()= default;

void
clientTcpSession::
run()
{
    // Read a request

    std::cout<<"Run Called TCP Session"<<std::endl;
    socket_.async_receive(tcpSessionStreamBuff.prepare(this->maxBufferSize),
                          [self = shared_from_this()]
            (errorCode ec, std::size_t bytes)
        {
            self->onRead(ec, bytes);
        });
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

void
clientTcpSession::
onRead(errorCode ec, std::size_t numbOfBytes)
{
    // Handle the error, if any
    if(ec)
        return fail(ec, "read");

    if (!ec)
    {
#ifdef LOG
        spdlog::info("Number Of bytes read {}", numbOfBytes);
#endif

        tcpSessionStreamBuff.commit(numbOfBytes);
        std::istream is(&tcpSessionStreamBuff);

        if (numbOfBytes > password.size() + 2) // 2 is added to compensate for 2 \n
        {
            char passChar[this->password.size() + 1]; //1 is added to null-terminate string
            extract(is, passChar, password.size() + 1);
            std::string pass(passChar);
            if (pass == password) {
#ifdef LOG
                spdlog::info("Password Received Matched");
#endif
                int bytesLeft = numbOfBytes - (password.size() + 1);
                char nameChar[bytesLeft];
                extract(is, nameChar, bytesLeft);
                std::string name(nameChar);
                std::make_shared<clientLobbySession>(
                        name,
                        std::move(socket_),
                        state)->run();
#ifdef LOG
                spdlog::info("Connection Promoted To Lobby-Session");
                spdlog::info("Name of the Player {}", name);
#endif
            }
#ifdef LOG
            else {
                spdlog::info("Connection Could not be promoted because of password mismatch");
                spdlog::info("Password Expected {}", password);
                spdlog::info("Password Received {}", pass);
            }
#endif
        }
#ifdef LOG
        else
        {
            spdlog::info("TCP connection not promoted and closed because of less number of bytes received");
            spdlog::info("Number of bytes should had been greater than", password.size() + 2);
            spdlog::info("Number of bytes received", numbOfBytes);
        }
#endif
        tcpSessionStreamBuff.consume(numbOfBytes);
    }
}