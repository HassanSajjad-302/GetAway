
#ifndef GETAWAY_RESOURCESTRINGS_HPP
#define GETAWAY_RESOURCESTRINGS_HPP

#include <string>
#include <mutex>

class resourceStrings {
public:
    static void clearAndPrint(const std::string& toPrint);
    static void clearAndPrint(const std::string &messageBuffer, const std::string &nonMessageBuffer,
                              const std::string &inputStatement, std::string& userIncomingInput,
                              std::mutex& mut, bool lock);
        static void print(const std::string& toPrint);
};


#endif //GETAWAY_RESOURCESTRINGS_HPP
