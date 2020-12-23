//Server

#ifdef ANDROID
#include "satiAndroid.hpp"
#else
#include "sati.hpp"
#endif
#include "serverAuthManager.hpp"
#include <fstream>
#include <memory>
#include "serverHome.hpp"
#include "constants.h"

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

#ifdef ANDROID
    sati::getInstanceFirstTime(io);
#else
    int a;
    std::mutex mu;
    std::thread inputThread{[s = std::ref(sati::getInstanceFirstTime(io, mu))](){s.get().operator()();}};
#endif
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

#ifndef ANDROID
    //inputThread.detach();
#endif
}
