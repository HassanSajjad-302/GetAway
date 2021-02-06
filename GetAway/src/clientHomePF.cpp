
#include "sati.hpp"
#include "constants.h"
#include "clientHome.hpp"

void clientHome::PF::setInputStatementHome7() {
    sati::getInstance()->nonMessageBuffer = gameStrings::clientHome;
}

void clientHome::PF::setInputStatementMAIN() {
    setInputStatementHome7();
    sati::getInstance()->accumulateBuffersAndPrint();
}

void clientHome::PF::setInputStatementHome7R1() {
    sati::getInstance()->nonMessageBuffer = gameStrings::enterRemoteServerIPAddress;
}

void clientHome::PF::setInputStatementIPADDRESS() {
    setInputStatementHome7R1();
    sati::getInstance()->accumulateBuffersAndPrint();
}

void clientHome::PF::setInputStatementHome7R2(const std::vector<std::tuple<std::string, std::string>>& servers) {
    assert(!servers.empty() && "Number Of Registered Server Should Not Be Zero");
    sati::getInstance()->nonMessageBuffer += "Please select one of the following\r\n";
    for(int i=0; i<servers.size(); ++i){
        sati::getInstance()->nonMessageBuffer += std::to_string(i+1) + ")\r\n";
        sati::getInstance()->nonMessageBuffer += "Server Name: " + std::get<0>(servers[i]) + "\r\n";
        sati::getInstance()->nonMessageBuffer += "Ip Address: " + std::get<1>(servers[i]) + "\r\n\n";
    }
}

void clientHome::PF::setInputStatementHome7R3(const std::vector<std::tuple<std::string, std::string>>& probeReply) {
    sati::getInstance()->nonMessageBuffer = "Press Enter To Cancel. Or The Listed Number To Join Server\r\n";
    int count = 1;
    for(auto& server:probeReply){
        sati::getInstance()->nonMessageBuffer += std::to_string(count) + ") \r\n";
        sati::getInstance()->nonMessageBuffer += "Server Name: " + std::get<0>(server) + "\r\n";
        sati::getInstance()->nonMessageBuffer += "Server Ip-Address " + std::get<1>(server) + "\r\n";
        ++count;
    }
    sati::getInstance()->accumulateBuffersAndPrint();
}

void clientHome::PF::setInputStatementClientName() {
    sati::getInstance()->nonMessageBuffer = gameStrings::enterYourName;
    sati::getInstance()->accumulateBuffersAndPrint();
}

void clientHome::PF::setInputStatementHomeGameRules(){
    sati::getInstance()->nonMessageBuffer = constants::gameRules;
    sati::getInstance()->nonMessageBuffer += "\r\nPress Enter To Go Back\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}

void clientHome::PF::setInputStatementHomeAbout(){
    sati::getInstance()->nonMessageBuffer = constants::about;
    sati::getInstance()->nonMessageBuffer += "\r\nPress Enter To Go Back\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}

void clientHome::PF::setInputStatementSELECTSERVER(const std::vector<std::tuple<std::string, std::string>>& servers){
    setInputStatementHome7R2(servers);
    sati::getInstance()->accumulateBuffersAndPrint();
}

void clientHome::PF::setInputStatementHome7R1RR() {
    sati::getInstance()->nonMessageBuffer = gameStrings::assignThisIpAddressAServerName;
}

void clientHome::PF::setInputStatementASSIGNSERVERNAME() {
    setInputStatementHome7R1RR();
    sati::getInstance()->accumulateBuffersAndPrint();
}

void clientHome::PF::setInputStatementConnectingToServer(const std::string& serverName){
    sati::getInstance()->nonMessageBuffer = "Connecting To Server " + serverName + ". To Cancel Connection Press 1\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}