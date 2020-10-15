#ifndef PACKET_PROCESSOR_HPP
#define PACKET_PROCESSOR_HPP

#include<string>
#include<boost/asio.hpp>

namespace net = boost::asio;

class Player_ProfileServer{
protected:
    std::string playerName;
public:
    Player_ProfileServer(std::string playerName){
        this->playerName = playerName;
    }
};

class Player_Profile_Client:protected Player_ProfileServer{
private:
    std::string remote_Password;
    Player_Profile_Client(std::string playerName, std::string remote_Password):
    Player_ProfileServer(playerName)
    {
        this->remote_Password = remote_Password;
    }
};


class packet_processor{
private:

};

#endif // PACKET_PROCESSOR_HPP
