
#ifndef GETAWAY_TERMINALINPUTBASE_HPP
#define GETAWAY_TERMINALINPUTBASE_HPP

#include "inputType.h"
#include <string>
class terminalInputBase{
public:
    virtual void input(std::string inputString, inputType inputReceivedType) =0;
};
#endif //GETAWAY_TERMINALINPUTBASE_HPP
