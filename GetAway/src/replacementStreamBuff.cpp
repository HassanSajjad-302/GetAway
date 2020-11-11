//
// Created by hassan on 11/11/20.
//

#include "replacementStreamBuff.hpp"

void replacementStreamBuff::evaluatePointers() {

    auto p1 = this->gptr();

        std::cout<<"Pointers Are Equal " << (this->gptr() == this->eback()) << std::endl;
        std::cout << "Get Pointer " << (void*)this->gptr() << std::endl;
        std::cout << "Pointer Beginning " << (void*)this->eback() << std::endl;
        std::cout << "Pointer Ending " << (void*) this->egptr()<<std::endl;
}

replacementStreamBuff::replacementStreamBuff() {
}
