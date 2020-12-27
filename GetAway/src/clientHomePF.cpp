//
// Created by hassan on 12/7/20.
//

#include "sati.hpp"
#include "clientHomePF.hpp"

void clientHomePF::setInputStatementHome7() {
    sati::getInstance()->inputStatementBuffer = "1)Add Server 2)Join Server 3)Find Local Server  4)Change Name "
                                                "5)Game Rules 6)Liscence 7)Exit\r\n";
}

void clientHomePF::setInputStatementMAIN() {
    setInputStatementHome7();
    sati::getInstance()->accumulateBuffersAndPrint();
}

void clientHomePF::setInputStatementHome7R1() {
    sati::getInstance()->inputStatementBuffer = "Please Enter Remote Server Ip-Address. Press Enter To Go Back.\r\n";
}

void clientHomePF::setInputStatementIPADDRESS() {
    setInputStatementHome7R1();
    sati::getInstance()->accumulateBuffersAndPrint();
}

void clientHomePF::setInputStatementHome7R2(const std::vector<std::tuple<std::string, std::string>>& servers) {
    assert(!servers.empty() && "Number Of Registered Server Should Not Be Zero");
    sati::getInstance()->inputStatementBuffer += "Please select one of the following\r\n";
    for(int i=0; i<servers.size(); ++i){
        sati::getInstance()->inputStatementBuffer += std::to_string(i+1) + ")\r\n";
        sati::getInstance()->inputStatementBuffer += "Ip Address: " + std::get<0>(servers[i]) + "\r\n";
        sati::getInstance()->inputStatementBuffer += "Server Name: " + std::get<1>(servers[i]) + "\r\n\n";
    }
}

void clientHomePF::setInputStatementHome7R3(const std::vector<std::tuple<std::string, std::string>>& probeReply) {
    sati::getInstance()->inputStatementBuffer = "Press Enter To Cancel. Or The Listed Number To Join Server\r\n";
    int count = 1;
    for(auto& server:probeReply){
        sati::getInstance()->inputStatementBuffer += std::to_string(count) + ") \r\n";
        sati::getInstance()->inputStatementBuffer += "Server Name: " + std::get<0>(server) + "\r\n";
        sati::getInstance()->inputStatementBuffer += "Server Ip-Address " + std::get<1>(server) + "\r\n";
        ++count;
    }
    sati::getInstance()->accumulateBuffersAndPrint();
}

void clientHomePF::setInputStatementHomeChangeName() {
    sati::getInstance()->inputStatementBuffer = "Please Enter Your Name. Press Enter To Cancel And Go Back.\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}

void clientHomePF::setInputStatementHomeGameRules(){
    sati::getInstance()->inputStatementBuffer = "No Game Rules. Press Enter To Go Back.\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}

void clientHomePF::setInputStatementHomeLiscence(){
    sati::getInstance()->inputStatementBuffer = "No Licence. Press Enter To Go Back.\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}

void clientHomePF::setInputStatementSELECTSERVER(const std::vector<std::tuple<std::string, std::string>>& servers){
    setInputStatementHome7R2(servers);
    sati::getInstance()->accumulateBuffersAndPrint();
}

void clientHomePF::setErrorMessageWrongIpAddress() {
    sati::getInstance()->inputStatementBuffer = "Enetered Ip Address Could Not Be Validated\r\n";
}

void clientHomePF::setErrorMessageWrongIpAddressAccumulate() {
    setErrorMessageWrongIpAddress();
    sati::getInstance()->accumulateBuffersAndPrint();
}

void clientHomePF::setInputStatementHome7R1RR() {
    sati::getInstance()->inputStatementBuffer = "Please Assign This Ip-Address A Server Name. Press Enter To Go Back.\r\n";
}

void clientHomePF::setInputStatementASSIGNSERVERNAME() {
    setInputStatementHome7R1RR();
    sati::getInstance()->accumulateBuffersAndPrint();
}

void clientHomePF::setInputStatementConnectingToServer(const std::string& serverName){
    sati::getInstance()->inputStatementBuffer = "Connecting To Server " + serverName + ". To Cancel Connection Press 1\r\n";
    sati::getInstance()->accumulateBuffersAndPrint();
}