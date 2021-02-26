
#ifndef GETAWAY_REPLACEMENTSTREAMBUFF_HPP
#define GETAWAY_REPLACEMENTSTREAMBUFF_HPP

#include "asio/streambuf.hpp"

class replacementStreamBuff : public asio::streambuf {
public:
    void evaluatePointers();
    replacementStreamBuff();
};


#endif //GETAWAY_REPLACEMENTSTREAMBUFF_HPP
