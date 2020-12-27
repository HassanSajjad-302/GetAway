#ifdef ANDROID

#include "../header/androidClientMain.hpp"
#include "sati.hpp"
#include "clientHome.hpp"
#include <functional>
androidClientMain::androidClientMain(asio::io_context &io_): io{io_} {}

void androidClientMain::run() {
    thr = std::thread([s = std::ref(io)](){
        sati::getInstanceFirstTime(s.get());

        clientHome h(s.get());
        h.run();

        s.get().run();

        //End Game Here Below This;
    });
}

androidClientMain::~androidClientMain() {

}
#endif