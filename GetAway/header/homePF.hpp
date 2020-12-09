//
// Created by hassan on 12/7/20.
//

#ifndef GETAWAY_HOMEPF_HPP
#define GETAWAY_HOMEPF_HPP


class homePF{
public:
    //input-statement-functions
    static void setInputStatementHome7();
    static void setInputStatementMAIN();

    static void setInputStatementHome7R1();
    static void setInputStatementIPADDRESS();

    static void setInputStatementHome7R2(const std::vector<std::tuple<std::string, std::string, std::string>>& servers);
    static void setInputStatementSELECTSERVER(const std::vector<std::tuple<std::string, std::string, std::string>>& servers);



    static void setErrorMessageWrongIpAddress();
    static void setErrorMessageWrongIpAddressAccumulate();

    static  void setInputStatementHome7R1R();
    static void setInputStatementPORTNUMBER();

    static  void setInputStatementHome7R1RR();
    static void setInputStatementASSIGNSERVERNAME();

    static void setErrorMessageWrongPortNumber();
    static void setErrorMessageWrongPortnNumberAccumulate();

    static void setInputStatementConnectingToServer(const std::string& serverName);
};



#endif //GETAWAY_HOMEPF_HPP
