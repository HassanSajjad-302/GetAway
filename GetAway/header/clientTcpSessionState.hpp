#ifndef GETAWAY_CLIENTTCPSESSIONSTATE_HPP
#define GETAWAY_CLIENTTCPSESSIONSTATE_HPP

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <iostream>

template <typename T>
class Session{};
// Represents the shared server state


class clientTcpSessionState
{
    std::string password;
    std::string playerName;



    friend std::ostream& operator<<(std::ostream& out, clientTcpSessionState& state){
        out << state.password << std::endl;
        out << state.playerName << std::endl;
        return out;
    }

public:
    explicit
    clientTcpSessionState(std::string playerName, std::string password);
    [[nodiscard]] int getClassWriteSize() const;
};

#endif //GETAWAY_CLIENTTCPSESSIONSTATE_HPP
