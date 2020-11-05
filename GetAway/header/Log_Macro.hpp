//
// Created by hassan on 10/20/20.
//

#ifndef GETAWAY_LOG_MACRO_HPP
#define GETAWAY_LOG_MACRO_HPP

#define LOG
#endif //GETAWAY_LOG_MACRO_HPP

#ifdef LOG
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#endif