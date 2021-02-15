//Server

#ifndef ANDROID

#include "sati.hpp"
#include <fstream>
#include <memory>
#include "serverHome.hpp"
#include "constants.h"

int main(){
#ifndef NDEBUG
    auto logger = spdlog::basic_logger_mt("MyLogger", "Logs.txt");
    std::ofstream{"Logs.txt",std::ios_base::app}<<"\n\n\n\n\nNewGame";
    spdlog::set_default_logger(logger);
    logger->flush_on(spdlog::level::info);
#endif


    asio::io_context io;

    int a;
    std::mutex mu;
    std::thread inputThread{[s = std::ref(sati::getInstanceFirstTime(io, mu))](){s.get().operator()();}};

    std::make_shared<serverHome>(serverHome(io))->run();
    /*serverHome h(io);
    h.run();*/


/*
    // Create and launch a listening port
    std::make_shared<serverListener>(
        io,
        tcp::endpoint{tcp::v4(), constants::PORT_SERVER_LISTENER},
        "password")->run();
*/
    io.run();
    inputThread.detach();
    constants::exitCookedTerminal();
}

#endif
