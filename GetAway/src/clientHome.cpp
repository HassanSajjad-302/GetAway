//
// Created by hassan on 12/8/20.
//

#include "clientHome.hpp"
#include "iostream"
#include "clientAuthManager.hpp"
#include <memory>
#include <regex>
clientHome::clientHome(net::io_context &io_): io(io_), guard(io.get_executor()), sock(io_){

}

void clientHome::run() {
    sati::getInstance()->setBase(this, appState::HOME);
    setInputType(inputType::HOMEMAIN);
    homePF::setInputStatementMAIN();
}

int clientHome::isValidIp4(const char *str) {
    int segs = 0;   /* Segment count. */
    int chcnt = 0;  /* Character count within segment. */
    int accum = 0;  /* Accumulator for segment. */

    /* Catch NULL pointer. */

    if (str == nullptr)
        return 0;

    /* Process every character in string. */

    while (*str != '\0') {
        /* Segment changeover. */

        if (*str == '.') {
            /* Must have some digits in segment. */

            if (chcnt == 0)
                return 0;

            /* Limit number of segments. */

            if (++segs == 4)
                return 0;

            /* Reset segment values and restart loop. */

            chcnt = accum = 0;
            str++;
            continue;
        }
        /* Check numeric. */

        if ((*str < '0') || (*str > '9'))
            return 0;

        /* Accumulate and check segment. */

        if ((accum = accum * 10 + *str - '0') > 255)
            return 0;

        /* Advance other segment specific stuff and continue loop. */

        chcnt++;
        str++;
    }

    /* Check enough segments and enough characters in last segment. */

    if (segs != 3)
        return 0;

    if (chcnt == 0)
        return 0;

    /* Address okay. */

    return 1;
}

void clientHome::input(std::string inputString, inputType inputReceivedType) {
    if(inputReceivedType == inputTypeExpected){
        if(inputReceivedType == inputType::HOMEMAIN){
            int input;
            if(inputHelper(inputString,
                           1, 7, inputType::HOMEMAIN, inputType::HOMEMAIN, input)){
                if(input == 1){
                    //Add Server
                    homePF::setInputStatementIPADDRESS();
                    setInputType(inputType::HOMEIPADDRESS);
                }else if(input == 2){
                    //Join Server
                    if(registeredServers.empty()){
                        setInputType(inputType::HOMEMAIN);
                        std::cout<<"No Registered Servers\r\n";
                    }else{
                        homePF::setInputStatementSELECTSERVER(registeredServers);
                        setInputType(inputType::HOMESELECTSERVER);
                    }
                }else if(input == 3){
                    //Change Name
                }else if(input == 4){
                    //Find Local Server
                }else if(input == 5){
                    //Game Rules
                }else if(input == 6){
                    //Liscence
                }else if(input == 7){
                    //Exit
                    if(sock.is_open()){
                        boost::system::error_code ec;
                        sock.shutdown(tcp::socket::shutdown_both, ec);
                        sock.close();
                    }
                    guard.reset();
                    system("stty cooked");
                }
            }
        }else if(inputReceivedType == inputType::HOMEIPADDRESS){
            if(isValidIp4(inputString.c_str())){
                ipAddress = std::move(inputString);
                homePF::setInputStatementPORTNUMBER();
                setInputType(inputType::HOMEPORTNUMBER);
            }else{
                std::cout<<"Please enter valid ip address"<<std::endl;
                setInputType(inputType::HOMEIPADDRESS);
            }
        }else if(inputReceivedType == inputType::HOMEPORTNUMBER){
            std::regex rg("^([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$");
            if(std::regex_match(inputString, rg)){
                portNumber = std::move(inputString);
                homePF::setInputStatementASSIGNSERVERNAME();
                setInputType(inputType::HOMEASSIGNSERVERNAME);
            }else{
                std::cout<<"Please enter valid port number"<<std::endl;
                setInputType(inputType::HOMEPORTNUMBER);
            }
        }else if(inputReceivedType == inputType::HOMEASSIGNSERVERNAME){
            registeredServers.emplace_back(std::move(ipAddress), std::move(portNumber), std::move(inputString));
            homePF::setInputStatementMAIN();
            setInputType(inputType::HOMEMAIN);
        }else if(inputReceivedType == inputType::HOMESELECTSERVER){
            int input;
            assert(!registeredServers.empty());
            if(inputHelper(inputString, 1, registeredServers.size(), inputType::HOMESELECTSERVER,
                           inputType::HOMESELECTSERVER, input)){
                --input;
                sock.async_connect(tcp::endpoint(make_address_v4(std::get<0>(registeredServers[input])),
                                                 std::stoi(std::get<1>(registeredServers[input]))),
                                                 [self = this](boost::system::error_code ec){
                    if(ec){
                        self->CONNECTTOSERVERFail(ec);
                    }else{
                        self->promote();
                    }
                });
                homePF::setInputStatementConnectingToServer(std::get<2>(registeredServers[input]));
                setInputType(inputType::HOMECONNECTTINGOSERVER);
            }
        }else if(inputReceivedType == inputType::HOMECONNECTTINGOSERVER){
            int input;
            if(inputHelper(inputString, 1, 1,inputType::HOMECONNECTTINGOSERVER,
                           inputType::HOMECONNECTTINGOSERVER,input)){
                boost::system::error_code ec;
                sock.shutdown(tcp::socket::shutdown_both, ec);
                sock.close();
            }
        }
    }else{
        std::cout<<"Unexpected input type input received"<<std::endl;
    }
}

void clientHome::CONNECTTOSERVERFail(boost::system::error_code ec){
    std::cout<<ec<<std::endl;
    homePF::setInputStatementMAIN();
    setInputType(inputType::HOMEMAIN);
    std::cout<<"Connect To Server Failed " <<std::endl;
}

bool clientHome::inputHelper(const std::string& inputString, int lower, int upper, inputType notInRange_,
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

void clientHome::setInputType(inputType type) {
    sati::getInstance()->setInputType(type);
    inputTypeExpected = type;
}

void clientHome::promote() {
    std::make_shared<session<clientAuthManager>>(std::move(sock),std::make_shared<clientAuthManager>(
            std::move(myName), "password"))->registerSessionToManager();
    guard.reset();
}
