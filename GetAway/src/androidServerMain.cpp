#ifdef ANDROID
#include "../header/androidServerMain.hpp"
#include "sati.hpp"
#include "serverHome.hpp"
#include <functional>

androidServerMain::androidServerMain(asio::io_context &io_): io{io_} {}

androidServerMain::~androidServerMain() {

}
#endif