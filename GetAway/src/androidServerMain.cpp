#ifdef ANDROID
#include "../header/androidServerMain.hpp"
#include "sati.hpp"
#include "serverHome.hpp"
#include <functional>

androidServerMain::androidServerMain(asio::io_context &io_): io{io_} {}

void androidServerMain::run() {
    thr = std::thread([s = std::ref(io)](){
        sati::getInstanceFirstTime(s.get());

        serverHome h(s.get());
        h.run();

        s.get().run();

        //End Game Here Below This;
    });
}

androidServerMain::~androidServerMain() {

}
#endif