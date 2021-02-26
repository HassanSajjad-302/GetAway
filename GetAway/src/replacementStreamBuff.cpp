
#include <iostream>
#include "replacementStreamBuff.hpp"
#include "resourceStrings.hpp"

void replacementStreamBuff::evaluatePointers() {

    auto p1 = this->gptr();

    resourceStrings::print("Pointers Are Equal " +
                           std::to_string((this->gptr() == this->eback())) + "\r\n");

    //TODO
    //Following Are Not Android Ready
        std::cout << "" << (void*)this->gptr() << std::endl;
        std::cout << "Pointer Beginning " << (void*)this->eback() << std::endl;
        std::cout << "Pointer Ending " << (void*) this->egptr()<<std::endl;
}

replacementStreamBuff::replacementStreamBuff() {
}
