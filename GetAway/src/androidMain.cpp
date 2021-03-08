
#ifdef ANDROID

#include "androidMain.hpp"

androidMain::androidMain(asio::io_context &io_): io{io_} {}

androidMain::~androidMain() = default;
#endif