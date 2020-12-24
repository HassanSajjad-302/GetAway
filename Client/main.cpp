//Client

#include "clientHome.hpp"
#include "sati.hpp"
#include "session.hpp"
#include "clientAuthManager.hpp"
#include "constants.h"
#include <fstream>
#include <memory>
#include <random>

//Following link can be helpful
//https://www.tutorialspoint.com/Read-a-character-from-standard-input-without-waiting-for-a-newline-in-Cplusplus

#ifdef ANDROID
void run(){
#else
int main(){
#endif
#if !defined(NDEBUG) && !defined(ANDROID)
    auto logger = spdlog::basic_logger_mt("MyLogger", "Logs.txt");
    std::ofstream{"Logs.txt",std::ios_base::app}<<"\n\n\n\n\nNewGame";
    spdlog::set_default_logger(logger);
    logger->flush_on(spdlog::level::info);
#endif
    asio::io_context io;
    std::mutex mu;
    std::thread inputThread{[s = std::ref(sati::getInstanceFirstTime(io, mu))](){s.get().operator()();}};

    /*tcp::endpoint endpoint(tcp::v4(),constants::PORT);
    tcp::socket sock(io);
    sock.connect(endpoint);
    sock.set_option(boost::asio::ip::tcp::no_delay(true));   // enable PSH
    //sock.set_option(boost::asio::ip::tcp::)
    //todo
    //temp testing change it later;
    std::default_random_engine rd{1};
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(1,200);
    std::string name = "Hassan ";
    name += std::to_string(dist(mt));
    std::make_shared<session<clientAuthManager>>(
            std::move(sock),
            std::make_shared<clientAuthManager>(std::move(name), "password"))->registerSessionToManager();*/





    clientHome h(io);
    h.run();


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

    inputThread.detach();
}
