#include "serverTcpSession.hpp"
#include "serverLobbySession.hpp"
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

serverTcpSession::
serverTcpSession(
    tcp::socket socket,
    std::shared_ptr<serverTcpSessionState>  state)
    : sock(std::move(socket))
    , state(std::move(state))
{
}
serverTcpSession::~serverTcpSession()= default;

//TODO
//Currently, it is assumed that password is small and is sent in one
//message with the name of the client player. For the messages that
//be of any length, it is not good assumption because message may
//arrive in pieces because of being a large message. This possibility
//is not ruled out in following scenario. Thus, we will have to
//implement some text wrapping which will tell us about the message
//limits and we will read up-until then.
//But this I will do after implementing the lobbysession which
//itself will be implemented through following mechanism first and
//tested for smaller messages and then updated to the text-wrapping
//mechanism later.

void
serverTcpSession::
run()
{
    sock.async_receive(serverTcpSessionStreamBuff.prepare(this->state->getClassReceiveSize()),
                       [self = shared_from_this()]
            (errorCode ec, std::size_t bytes)
        {
            self->onRead(ec, bytes);
        });
}

// Report a failure
void
serverTcpSession::
fail(errorCode ec, char const* what)
{
    // Don't report on canceled operations
    if(ec == net::error::operation_aborted)
        return;

    std::cerr << what << ": " << ec.message() << "\n";
}

void
serverTcpSession::
onRead(errorCode ec, std::size_t numbOfBytes)
{
    // Handle the error, if any
    if(ec)
        return fail(ec, "read");


    if (!ec)
    {
        serverTcpSessionStreamBuff.commit(numbOfBytes);
        if (numbOfBytes > state->getMinimumReceivedBytes()) // 2 is added to compensate for 2 \n
        {
            is >> *state;
            //If password does not match, connection will be closed.
            if(state->getPasswordMatched()){
                std::make_shared<serverLobbySession>(
                        name,
                        std::move(sock),
                        state)->run();
            }
        }
        serverTcpSessionStreamBuff.consume(numbOfBytes);
    }
}