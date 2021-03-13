//Server

#ifndef ANDROID
//To test uncomment following line. And compile other cmake target clientTest.
//This target will directly start server while clientTest will bind with that local server. Thus saving us
//some steps and bringing us quickly to the debugging core game.
//#define TESTING 1

#include "sati.hpp"
#include <fstream>
#include <memory>
#ifdef TESTING
#include <serverListener.hpp>
#endif
#include "constants.hpp"
#include "home.hpp"
int main(){
#ifndef NDEBUG
    auto logger = spdlog::basic_logger_mt("MyLogger", "Logs.txt");
    std::ofstream{"Logs.txt",std::ios_base::app}<<"\n\n\n\n\nNewGame";
    spdlog::set_default_logger(logger);
    logger->flush_on(spdlog::level::info);
#endif


    asio::io_context io;

    std::mutex mu;
    std::thread inputThread{[s = std::ref(sati::getInstanceFirstTime(io, mu))](){s.get().operator()();}};
#ifdef TESTING
    std::make_shared<serverListener>(
                                io,
                                tcp::endpoint{tcp::v4(), constants::PORT_SERVER_LISTENER},
                                "Server", "Player", constants::gamesEnum::BLUFF)->run();
#else
    std::make_shared<home>(home(io))->run();
#endif
    io.run();
    inputThread.detach();
    constants::exitCookedTerminal();
}

#endif
