//
// Created by hassan on 12/19/2020.
//

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

void resourceStrings::print(const std::string &toPrint) {
    std::cout<<toPrint;
}
#endif

