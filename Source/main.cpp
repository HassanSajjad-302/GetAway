//
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/CppCon2018
//

//------------------------------------------------------------------------------
/*
    WebSocket chat server

    This implements a multi-user chat room using WebSocket.
*/
//------------------------------------------------------------------------------

#include "listener.hpp"
#include "shared_state.hpp"
#include <boost/asio/signal_set.hpp>
#include <iostream>
#include <thread>
#include <boost/config.hpp>
#include <memory>

std::string get_compile_version()
{
     char buffer[sizeof(BOOST_PLATFORM) + sizeof(BOOST_COMPILER) +sizeof(__DATE__ )+ 5];
     sprintf(buffer, "[%s/%s](%s)", BOOST_PLATFORM, BOOST_COMPILER, __DATE__);
     std::string compileinfo(buffer);
     return compileinfo;
}


#define Utility
#undef Utility

int
main(int argc, char* argv[])
{

    std::cout<<get_compile_version();
    std::cout<<std::endl;


#ifdef Utility

    // Check command line arguments.
     if (argc != 3)
     {
         std::cerr <<
             "Usage: websocket-chat-server <address> <port> <doc_root>\n" <<
             "Example:\n" <<
             "    websocket-chat-server 0.0.0.0 8080 .\n";
         return EXIT_FAILURE;
     }

    auto address = net::ip::make_address(argv[1]);
    auto port = static_cast<unsigned short>(std::atoi(argv[2]));

#else
    auto address = net::ip::make_address("127.0.0.1");
    auto port = static_cast<unsigned short>(std::atoi("3000"));
#endif
    // The io_context is required for all I/O
    net::io_context ioc;

    // Create and launch a listening port
    std::make_shared<listener>(
        ioc,
                tcp::endpoint{tcp::v4(), port},
        std::make_shared<shared_state>())->run();

    // Capture SIGINT and SIGTERM to perform a clean shutdown
    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(
        [&ioc](boost::system::error_code const&, int)
        {
            // Stop the io_context. This will cause run()
            // to return immediately, eventually destroying the
            // io_context and any remaining handlers in it.
            ioc.stop();
        });

    // Run the I/O service on the main thread
    ioc.run();

    // (If we get here, it means we got a SIGINT or SIGTERM)

    return EXIT_SUCCESS;
}
