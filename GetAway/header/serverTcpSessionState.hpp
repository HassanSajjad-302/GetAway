#ifndef CPPCON2018_SHARED_STATE_HPP
#define CPPCON2018_SHARED_STATE_HPP

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <iostream>
class serverLobbySession;

// Represents the shared server state
class serverTcpSessionState
{
    std::string password;
    std::string playerName;
    int classSendSize =0;
    bool passwordMatched = false;
//TODO
//Following constant should be supplied from somewhere else.
    const int nameLength = 60; //Maximum chars for name

    friend std::istream& operator>>(std::istream& in, serverTcpSessionState& state){
        char arr[61]; //This constant will be fed from somewhere else but one is added.
        in.getline(arr,61);
        std::string str(arr);
        if(str == state.password)
        {
            in.getline(arr,61);
            str = std::string(arr);
            state.playerName = std::move(str);
            state.passwordMatched = true;
        }
        else
        {
            state.passwordMatched = false;
        }
        return in;
    }


public:
    explicit
    serverTcpSessionState(std::string password);

    [[nodiscard]] int getClassReceiveSize() const;
    [[nodiscard]] int getClassSendSize() const;
    [[nodiscard]] int getMinimumReceivedBytes() const;
    [[nodiscard]] bool getPasswordMatched() const;
    void setClassSendSize(int size);

};

#endif
