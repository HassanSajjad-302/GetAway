//Server

#ifndef ANDROID

#define TESTING 1

#include "sati.hpp"
#include <fstream>
#include <memory>
#ifdef TESTING
#include "clientLobby.hpp"
#endif
#include "constants.h"
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

    tcp::endpoint endpoint(tcp::v4(),3000);
    tcp::socket sock(io);
    sock.connect(endpoint);
#ifdef TESTING
    std::make_shared<clientSession<clientLobby, asio::io_context&, std::string, serverListener*, bool, constants::gamesEnum>>(
            std::move(sock), io, "Player42", nullptr, true, constants::gamesEnum::BLUFF)->run();
#else
    std::make_shared<home>(io)->run();
#endif
    io.run();
    inputThread.detach();
    constants::exitCookedTerminal();
}

#endif
