
#include <asio.hpp>
#include "GetAwayTest.hpp"

class Cat{
public:
    void outputReceivedFromInferior(std::string str){
        std::cout<<str<<std::endl;
    }
};
int main(){
    /*asio::io_context io;
    GetAwayTest a(io);
    io.run();*/
    constexpr static const char serverApp[] = "../GetAway",
            clientApp[] = "../Client/Client", app[] = "../app";
    asio::io_context io;
    Cat m;
    inferior<Cat, false, app, app> a(&m, io);
    a.receiveOutputFromInferior();
    io.run();
}