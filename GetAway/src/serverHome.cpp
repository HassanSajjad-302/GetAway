
#include "resourceStrings.hpp"
#include "serverHome.hpp"
#include "serverPF.hpp"
#include "serverAuthManager.hpp"
#include "sati.hpp"
#include <memory>
#include <regex>

serverHome::serverHome(asio::io_context &io_): io(io_), guard(io.get_executor()), sock(io_){

}

void serverHome::run() {
    sati::getInstance()->setBase(this, appState::HOME);
    setInputType(inputType::HOMEMAIN);
    serverPF::setHomeMain();
    ref = this->shared_from_this();
}

void serverHome::input(std::string inputString, inputType inputReceivedType) {
    if(inputReceivedType == inputTypeExpected){
        if(inputReceivedType == inputType::HOMEMAIN){
            int input;
            if(constants::inputHelper(inputString,
                           1, 2, inputType::HOMEMAIN, inputType::HOMEMAIN, input)){
                if(input == 1){
                    //Change Server Name
                    serverPF::setHomeChangeServerName();
                    setInputType(inputType::HOMESTARTSERVER);
                }else if(input == 2){
                    //Exit
                    if(sock.is_open()){
                        asio::error_code ec;
                        sock.shutdown(tcp::socket::shutdown_both, ec);
                        sock.close();
                    }
                    guard.reset();
                    ref.reset();
                    constants::exitCookedTerminal();
                }
            }
        }if(inputReceivedType == inputType::HOMESTARTSERVER){
            if(inputString.empty()){
                std::make_shared<serverListener>(
                        io,
                        tcp::endpoint{tcp::v4(), constants::PORT_SERVER_LISTENER},
                        "Server",
                        "password")->run();
                guard.reset();
                ref.reset();
            }else{
                std::make_shared<serverListener>(
                        io,
                        tcp::endpoint{tcp::v4(), constants::PORT_SERVER_LISTENER},
                        inputString,
                        "password")->run();
                guard.reset();
                ref.reset();
            }
        }
    }else{
        resourceStrings::print("Unexpected input type input received\r\n");
    }
}

void serverHome::setInputType(inputType type) {
    sati::getInstance()->setInputType(std::make_tuple(ith::HOME, type));
    inputTypeExpected = type;
}