//
// Created by hassan on 11/11/20.
//

#ifndef GETAWAY_REPLACEMENTSTREAMBUFF_HPP
#define GETAWAY_REPLACEMENTSTREAMBUFF_HPP

#include "boost/asio.hpp"
#include <iostream>
namespace net = boost::asio;

class replacementStreamBuff : public net::streambuf {
public:
    void evaluatePointers();
    replacementStreamBuff();
};


#endif //GETAWAY_REPLACEMENTSTREAMBUFF_HPP
