//
// Created by hassan on 12/9/20.
//

#include "serverHome.hpp"
#include "serverPF.hpp"
#include "iostream"
#include "serverAuthManager.hpp"
#include <memory>
#include <regex>

serverHome::serverHome(net::io_context &io_): io(io_), guard(io.get_executor()), sock(io_){

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
                    std::make_shared<serverListener>(
                            io,
                            tcp::endpoint{tcp::v4(), port},
                            "password")->run();
                    guard.reset();
                }else if(input == 2){
                    //Change Port
                    serverPF::setHomeChangePort();
                    setInputType(inputType::HOMEPORTNUMBER);
                }else if(input == 3){
                    //Exit
                    if(sock.is_open()){
                        boost::system::error_code ec;
                        sock.shutdown(tcp::socket::shutdown_both, ec);
                        sock.close();
                    }
                    guard.reset();
#ifdef __linux__
                    system("stty cooked");
#endif
                }
            }
        }else if(inputReceivedType == inputType::HOMEPORTNUMBER){
            std::regex rg("^([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$");
            if(std::regex_match(inputString, rg)){
                port = static_cast<unsigned short>(std::stoi(inputString));
                serverPF::setHomeMain();
                setInputType(inputType::HOMEMAIN);
            }else{
                std::cout<<"Please enter valid port number"<<std::endl;
                setInputType(inputType::HOMEPORTNUMBER);
                serverPF::setErrorMessageWrongPortNumber();
            }
        }
    }else{
        std::cout<<"Unexpected input type input received\r\n";
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
            std::cout<<"Please enter integer in range \r"<<std::endl;
            return false;
        }
    }
    catch (std::invalid_argument& e) {
        sati::getInstance()->accumulatePrint();
        sati::getInstance()->setInputType(invalidInput_);
        std::cout<<"Invalid Input. \r"<<std::endl;
        return false;
    }
}

void serverHome::setInputType(inputType type) {
    sati::getInstance()->setInputType(type);
    inputTypeExpected = type;
}