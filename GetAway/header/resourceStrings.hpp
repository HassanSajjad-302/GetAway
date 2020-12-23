//
// Created by hassan on 12/19/2020.
//

#ifndef GETAWAY_RESOURCESTRINGS_HPP
#define GETAWAY_RESOURCESTRINGS_HPP

#include <string>

class resourceStrings {
public:
    const static inline std::string IPADDRESSNOTVALIDATED = "Your Ip-Address Could Not Be Validated\r\n";

    static void clearAndPrint(const std::string& toPrint);
    static void print(const std::string& toPrint);
};


#endif //GETAWAY_RESOURCESTRINGS_HPP
