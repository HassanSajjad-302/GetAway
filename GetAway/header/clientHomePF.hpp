//
// Created by hassan on 12/7/20.
//

#ifndef GETAWAY_CLIENTHOMEPF_HPP
#define GETAWAY_CLIENTHOMEPF_HPP


class clientHomePF{
public:
    //input-statement-functions
    static void setInputStatementHome7();
    static void setInputStatementMAIN();

    static void setInputStatementHome7R1();
    static void setInputStatementIPADDRESS();

    static void setInputStatementHome7R2(const std::vector<std::tuple<std::string, std::string>>& servers);
    static void setInputStatementSELECTSERVER(const std::vector<std::tuple<std::string, std::string>>& servers);

    static void setInputStatementHome7R3(const std::vector<std::tuple<std::string, std::string>>& probeReply);

    static void setInputStatementClientName();
    static void setInputStatementHomeGameRules();
    static void setInputStatementHomeAbout();

    static void setErrorMessageWrongIpAddress();
    static void setErrorMessageWrongIpAddressAccumulate();

    static  void setInputStatementHome7R1RR();
    static void setInputStatementASSIGNSERVERNAME();

    static void setInputStatementConnectingToServer(const std::string& serverName);
};



#endif //GETAWAY_CLIENTHOMEPF_HPP
