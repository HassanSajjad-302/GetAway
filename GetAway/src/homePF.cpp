//
// Created by hassan on 12/7/20.
//

#include <sati.hpp>
#include "homePF.hpp"

void homePF::setInputStatementHome7() {
    sati::getInstance()->inputStatementBuffer = "1)Add Server 2)Join Server 3)Change Name "
                                                "4)Find Local Server 5)Game Rules 6)Liscence 7)Exit\r\n";
}

void homePF::setInputStatementMAIN() {
    setInputStatementHome7();
    sati::getInstance()->accumulateBuffersAndPrint(true);
}

void homePF::setInputStatementHome7R1() {
    sati::getInstance()->inputStatementBuffer = "Please Enter Remote Server Ip-Address\r\n";
}

void homePF::setInputStatementIPADDRESS() {
    setInputStatementHome7R1();
    sati::getInstance()->accumulateBuffersAndPrint(true);
}

void homePF::setInputStatementHome7R2(const std::vector<std::tuple<std::string, std::string, std::string>>& servers) {
    assert(!servers.empty() && "Number Of Registered Server Should Not Be Zero");
    sati::getInstance()->inputStatementBuffer += "Please select one of the following\r\n";
    for(int i=0; i<servers.size(); ++i){
        sati::getInstance()->inputStatementBuffer += std::to_string(i+1) + ")\r\n";
        sati::getInstance()->inputStatementBuffer += "Ip Address: " + std::get<0>(servers[i]) + "\r\n";
        sati::getInstance()->inputStatementBuffer += "Port Number: " + std::get<1>(servers[i]) + "\r\n";
        sati::getInstance()->inputStatementBuffer += "Server Name: " + std::get<2>(servers[i]) + "\r\n\n";
    }
}

void homePF::setInputStatementSELECTSERVER(const std::vector<std::tuple<std::string, std::string, std::string>>& servers){
    setInputStatementHome7R2(servers);
    sati::getInstance()->accumulateBuffersAndPrint(true);
}

void homePF::setErrorMessageWrongIpAddress() {
    sati::getInstance()->inputStatementBuffer = "Enetered Ip Address Could Not Be Validated\r\n";
}

void homePF::setErrorMessageWrongIpAddressAccumulate() {
    setErrorMessageWrongIpAddress();
    sati::getInstance()->accumulateBuffersAndPrint(true);
}

void homePF::setInputStatementHome7R1R() {
    sati::getInstance()->inputStatementBuffer = "Please Enter Remote Server Port Number\r\n";
}

void homePF::setInputStatementPORTNUMBER() {
    setInputStatementHome7R1R();
    sati::getInstance()->accumulateBuffersAndPrint(true);
}

void homePF::setInputStatementHome7R1RR() {
    sati::getInstance()->inputStatementBuffer = "Please Assign This Ip-Address And Port Number A Server Name\r\n";
}

void homePF::setInputStatementASSIGNSERVERNAME() {
    setInputStatementHome7R1RR();
    sati::getInstance()->accumulateBuffersAndPrint(true);
}

void homePF::setErrorMessageWrongPortNumber() {
    sati::getInstance()->inputStatementBuffer = "Entered Port Number Could Not Be Validated\r\n";
}

void homePF::setErrorMessageWrongPortnNumberAccumulate() {
    setErrorMessageWrongPortNumber();
    sati::getInstance()->accumulateBuffersAndPrint(true);
}

void homePF::setInputStatementConnectingToServer(const std::string& serverName){
    sati::getInstance()->inputStatementBuffer = "Connecting To Server " + serverName + ". To Cancel Connection Press 1\r\n";
    sati::getInstance()->accumulateBuffersAndPrint(true);
}