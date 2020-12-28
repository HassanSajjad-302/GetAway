#ifdef ANDROID

#include "../header/androidClientMain.hpp"
#include "sati.hpp"
#include "clientHome.hpp"
#include <functional>
#include <memory>
androidClientMain::androidClientMain(asio::io_context &io_): io{io_} {}

androidClientMain::~androidClientMain() {

}
#endif