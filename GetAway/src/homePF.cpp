
#include "sati.hpp"
#include "constants.h"
#include "home.hpp"

void home::PF::setInputStatementHome7() {
    sati::getInstance()->inputStatement = "1)Start Server 2)Add Server 3)Join Server 4)Find Local Server"
                                                " 5)Game Rules 6)About 7)Exit\r\n";
}

void home::PF::setHomeChangeServerName(){
    sati::getInstance()->inputStatement = "Enter Server Name. Press Enter To Use Default.\r\n";
    sati::getInstance()->accumulatePrint();
}

void home::PF::setInputStatementMAIN() {
    setInputStatementHome7();
    sati::getInstance()->accumulatePrint();
}

void home::PF::setInputStatementHome7R1() {
    sati::getInstance()->inputStatement = "Please Enter Remote Server Ip-Address. Press Enter To Go Back.\r\n";
}

void home::PF::setInputStatementIPADDRESS() {
    setInputStatementHome7R1();
    sati::getInstance()->accumulatePrint();
}


void home::PF::setInputStatementHome7R2(const std::vector<std::tuple<std::string, std::string>>& servers) {
    assert(!servers.empty() && "Number Of Registered Server Should Not Be Zero");
    sati::getInstance()->inputStatement += "Please select one of the following\r\n";
    for(int i=0; i<servers.size(); ++i){
        sati::getInstance()->inputStatement += std::to_string(i+1) + ")\r\n";
        sati::getInstance()->inputStatement += "Server Name: " + std::get<0>(servers[i]) + "\r\n";
        sati::getInstance()->inputStatement += "Ip Address: " + std::get<1>(servers[i]) + "\r\n\n";
    }
}

void home::PF::setInputStatementHome7R3(const std::vector<std::tuple<std::string, std::string>>& probeReply) {
    sati::getInstance()->inputStatement = "Press Enter To Cancel. Or The Listed Number To Join Server\r\n";
    int count = 1;
    for(auto& server:probeReply){
        sati::getInstance()->inputStatement += std::to_string(count) + ") \r\n";
        sati::getInstance()->inputStatement += "Server Name: " + std::get<0>(server) + "\r\n";
        sati::getInstance()->inputStatement += "Server Ip-Address " + std::get<1>(server) + "\r\n";
        ++count;
    }
    sati::getInstance()->accumulatePrint();
}

void home::PF::setInputStatementClientName() {
    sati::getInstance()->inputStatement = "Please Enter Your Name. Press Enter To Use Default.\r\n";
    sati::getInstance()->accumulatePrint();
}

void home::PF::setInputStatementSELECTGAME() {
    sati::getInstance()->inputStatement = "Pleas Enter Corresponding Number To Select The Game.\r\n";
    for(int i=0; i<constants::NUMBER_OF_GAMES; ++i){
        sati::getInstance()->inputStatement += std::to_string(i+1) + ") " + constants::gamesNames[i] + "\t";
    }
    sati::getInstance()->inputStatement += "\r\n";
    sati::getInstance()->accumulatePrint();
}

void home::PF::setInputStatementHomeGameRules(){
    sati::getInstance()->nonMessageBuffer = constants::gameRules;
    sati::getInstance()->nonMessageBuffer += "\r\nPress Enter To Go Back\r\n";
    sati::getInstance()->accumulatePrint();
}

void home::PF::setInputStatementHomeAbout(){
    sati::getInstance()->inputStatement = constants::about;
    sati::getInstance()->inputStatement += "\r\nPress Enter To Go Back\r\n";
    sati::getInstance()->accumulatePrint();
}

void home::PF::setInputStatementSELECTSERVER(const std::vector<std::tuple<std::string, std::string>>& servers){
    setInputStatementHome7R2(servers);
    sati::getInstance()->accumulatePrint();
}

void home::PF::setInputStatementHome7R1RR() {
    sati::getInstance()->inputStatement = "Please Assign This Ip-Address A Server Name. Press Enter To Go Back.\r\n";
}

void home::PF::setInputStatementASSIGNSERVERNAME() {
    setInputStatementHome7R1RR();
    sati::getInstance()->accumulatePrint();
}

void home::PF::setInputStatementConnectingToServer(const std::string& serverName){
    sati::getInstance()->inputStatement = "Connecting To Server " + serverName + ". To Cancel Connection Press 1\r\n";
    sati::getInstance()->accumulatePrint();
}