//
// Created by hassan on 12/9/20.
//

#include "resourceStrings.hpp"
#include "serverHome.hpp"
#include "serverPF.hpp"
#include "serverAuthManager.hpp"
#include <memory>
#include <regex>

serverHome::serverHome(asio::io_context &io_): io(io_), guard(io.get_executor()), sock(io_){

}

void serverHome::run() {
    sati::getInstance()->setBase(this, appState::HOME);
    setInputType(inputType::HOMEMAIN);
    serverPF::setHomeMain();
}

void serverHome::input(std::string inputString, inputType inputReceivedType) {
    if(inputReceivedType == inputTypeExpected){
        if(inputReceivedType == inputType::HOMEMAIN){
            int input;
            if(inputHelper(inputString,
                           1, 3, inputType::HOMEMAIN, inputType::HOMEMAIN, input)){
                if(input == 1){
                    //Change Server Name
                    serverPF::setHomeChangeServerName();
                    setInputType(inputType::HOMESTARTSERVER);
                }else if(input == 3){
                    //Exit
                    if(sock.is_open()){
                        asio::error_code ec;
                        sock.shutdown(tcp::socket::shutdown_both, ec);
                        sock.close();
                    }
                    guard.reset();
#ifdef __linux__
                    system("stty cooked");
#endif
                }
            }
        }if(inputReceivedType == inputType::HOMESTARTSERVER){
            std::make_shared<serverListener>(
                    io,
                    tcp::endpoint{tcp::v4(), constants::PORT},
                    inputString,
                    "password")->run();
            guard.reset();
        }
    }else{
        resourceStrings::print("Unexpected input type input received\r\n");
    }
}

bool serverHome::inputHelper(const std::string& inputString, int lower, int upper, inputType notInRange_,
                             inputType invalidInput_, int& input_){
    try{
        int num = std::stoi(inputString);
        if(num>=lower && num<=upper){
            input_ = num;
            return true;
        }else{
            sati::getInstance()->accumulatePrint();
            sati::getInstance()->setInputType(notInRange_);
            resourceStrings::print("Please enter integer in range\r\n");
            return false;
        }
    }
    catch (std::invalid_argument& e) {
        sati::getInstance()->accumulatePrint();
        sati::getInstance()->setInputType(invalidInput_);
        resourceStrings::print("Invalid Input\r\n");
        return false;
    }
}

void serverHome::setInputType(inputType type) {
    sati::getInstance()->setInputType(type);
    inputTypeExpected = type;
}