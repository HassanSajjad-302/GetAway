//Client

#ifndef ANDROID
#include "clientHome.hpp"
#include "sati.hpp"
#include "constants.h"
#include <fstream>
#include <memory>

//Following link can be helpful
//https://www.tutorialspoint.com/Read-a-character-from-standard-input-without-waiting-for-a-newline-in-Cplusplus


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

    /*tcp::endpoint endpoint(tcp::v4(),constants::PORT_SERVER_LISTENER);
    tcp::socket sock(io);
    sock.connect(endpoint);
    sock.set_option(asio::ip::tcp::no_delay(true));   // enable PSH
    //sock.set_option(boost::asio::ip::tcp::)
    std::default_random_engine rd{1};
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(1,200);
    std::string name = "Hassan ";
    name += std::to_string(dist(mt));
    std::make_shared<session<clientAuthManager>>(
            std::move(sock),
            std::make_shared<clientAuthManager>(std::move(name), "password"))->registerSessionToManager();*/





    std::make_shared<clientHome>(clientHome(io))->run();

    io.run();

    inputThread.detach();
    constants::exitCookedTerminal();

}
#endif
