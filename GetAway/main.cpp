//Server

#include "sati.hpp"
#include "serverAuthManager.hpp"
#include <fstream>
#include <iostream>
#include <memory>
#include "serverHome.hpp"

int
main(int argc, char* argv[])
{
    auto logger = spdlog::basic_logger_mt("MyLogger", "Logs.txt");
    std::ofstream{"Logs.txt",std::ios_base::app}<<"\n\n\n\n\nNewGame";
    spdlog::set_default_logger(logger);
    logger->flush_on(spdlog::level::info);


    net::io_context io;

    std::mutex mu;
    std::thread inputThread{[s = std::ref(sati::getInstanceFirstTime(io, mu))](){s.get().operator()();}};
    serverHome h(io);
    h.run();

/*
    unsigned short port = 3000;
    // Create and launch a listening port
    std::make_shared<serverListener>(
        io,
        tcp::endpoint{tcp::v4(), port},
        "password")->run();
*/
    io.run();
    inputThread.detach();
}
