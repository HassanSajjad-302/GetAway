
#include "resourceStrings.hpp"
#include <iostream>

//This function takes the string and supplies to the android textview.
//One of these will clear the textview first while other won't.
#ifndef ANDROID
void resourceStrings::clearAndPrint(const std::string &toPrint) {
#ifdef __linux__
    system("clear");
#endif
#if defined(_WIN32) || defined(_WIN64)
    system("cls");
#endif
    std::cout<<toPrint;
}

void resourceStrings::clearAndPrint(const std::string &messageBuffer, const std::string &nonMessageBuffer,
                                    const std::string &inputStatement, std::string& userIncomingInput,
                                    std::mutex& mut, bool lock){
#ifdef __linux__
    system("clear");
#endif
#if defined(_WIN32) || defined(_WIN64)
    system("cls");
#endif
    std::cout<<messageBuffer << nonMessageBuffer << inputStatement;
    if(lock){
        std::lock_guard lockGuard(mut);
        std::cout<<userIncomingInput;
    }else{
        std::cout<<userIncomingInput;
    }
}

void resourceStrings::print(const std::string &toPrint) {
    std::cout<<toPrint;
}
#endif
