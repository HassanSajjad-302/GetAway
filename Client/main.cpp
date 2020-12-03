//Client

#include "sati.hpp"
#include "session.hpp"
#include "clientAuthManager.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <fstream>
#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <random>


std::string getCompileVersion()
{
     char buffer[sizeof(BOOST_PLATFORM) + sizeof(BOOST_COMPILER) +sizeof(__DATE__ )+ 5];
     sprintf(buffer, "[%s/%s](%s)", BOOST_PLATFORM, BOOST_COMPILER, __DATE__);
     std::string compileInfo(buffer);
     return compileInfo;
}

//TODO
//Cosole output will be displaying the conversation until the point. When new message is received
//the cosole clears and prints the conversation with the current game state. We parse input char by char
//so that when new message is received, we can print the console with same input. However this may
//cause problem of not able to use backspace option on that input. And if we can clear the chars, then
//the problem of updating our char array in the programme.
//Following link can be helpful
//https://www.tutorialspoint.com/Read-a-character-from-standard-input-without-waiting-for-a-newline-in-Cplusplus

namespace net = boost::asio;
using namespace net::ip;

int
main(int argc, char* argv[])
{
    system("clear");

    auto logger = spdlog::basic_logger_mt("MyLogger", "Logs.txt");
    std::ofstream{"Logs.txt",std::ios_base::app}<<"\n\n\n\n\nNewGame";
    spdlog::set_default_logger(logger);
    logger->flush_on(spdlog::level::info);
    spdlog::info("Hassan Sajjad");
#define LOG
    net::io_context io;
    std::mutex mu;
    std::thread inputThread{[s = std::ref(sati::getInstanceFirstTime(io, mu))](){s.get().operator()();}};
    tcp::endpoint endpoint(tcp::v4(),3000);
    tcp::socket sock(io);
    sock.connect(endpoint);
    //todo
    //temp testing change it later;
    std::default_random_engine rd{1};
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(1,200);
    std::string name = "Hassan ";
    name += std::to_string(dist(mt));
    std::make_shared<session<clientAuthManager>>(
            std::move(sock),
            std::make_shared<clientAuthManager>(std::move(name), "password"))->registerSessionToManager();


   /* // Capture SIGINT and SIGTERM to perform a clean shutdown
    net::signal_set signals(io, SIGINT, SIGTERM);
    signals.async_wait(
        [&io](boost::system::error_code const&, int)
        {
            // Stop the io_context. This will cause run()
            // to return immediately, eventually destroying the
            // io_context and any remaining handlers in it.
            io.stop();
        });*/

    // Run the I/O service on the main thread
    io.run();

    spdlog::info("Reached Here");
    // (If we get here, it means we got a SIGINT or SIGTERM)
    inputThread.detach();
    return EXIT_SUCCESS;
}
