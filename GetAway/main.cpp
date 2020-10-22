//Main

#include "serverListener.hpp"
#include "serverTcpSessionState.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <fstream>
#include "clientTcpSessionState.hpp"
#include <boost/asio/signal_set.hpp>
#include <iostream>
#include <boost/config.hpp>
#include <memory>

std::string getCompileVersion()
{
     char buffer[sizeof(BOOST_PLATFORM) + sizeof(BOOST_COMPILER) +sizeof(__DATE__ )+ 5];
     sprintf(buffer, "[%s/%s](%s)", BOOST_PLATFORM, BOOST_COMPILER, __DATE__);
     std::string compileinfo(buffer);
     return compileinfo;
}

//TODO
//Cosole output will be displaying the conversation until the point. When new message is received
//the cosole clears and prints the conversation with the current game state. We parse input char by char
//so that when new message is received, we can print the console with same input. However this may
//cause problem of not able to use backspace option on that input. And if we can clear the chars, then
//the problem of updating our char array in the programme.
//Following link can be helpful
//https://www.tutorialspoint.com/Read-a-character-from-standard-input-without-waiting-for-a-newline-in-Cplusplus



int
main(int argc, char* argv[])
{

    auto logger = spdlog::basic_logger_mt("MyLogger", "Logs.txt");
    std::ofstream{"Logs.txt",std::ios_base::app}<<"\n\n\n\n\nNewGame";
    spdlog::set_default_logger(logger);
    logger->flush_on(spdlog::level::info);

    std::cout << getCompileVersion();
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
    std::make_shared<serverListener>(
        ioc,
                tcp::endpoint{tcp::v4(), port},
        std::make_shared<serverTcpSessionState>("password"))->run();

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
