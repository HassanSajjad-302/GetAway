
#include <random>
#include "GetAwayTest.hpp"
#include "asio.hpp"
#include "../../GetAway/header/constants.h"
GetAwayTest::GetAwayTest(asio::io_context &io_): io{io_}, serverInferior{this, io}, serverTimer{io}{
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(97, 122);
    std::string vec;
    vec.resize(5);
    clientNames.resize(5);
    for(int i=0; i<numberOfClients;++i){
        clientInferiors.emplace_back(this, i, io);

        for (int j = 0; j < 5; ++j){
            vec[j] = dist(mt);
        }
        clientNames[i] = vec;

        clientTimers.emplace_back(io);
        currentClientStages.emplace_back(clientStages::START);
        clientTimers[i].expires_from_now(std::chrono::seconds(waitForOutputSeconds));
        clientTimers[i].async_wait([this, id = i](std::error_code ec){
            this->clientTimerExpiredBeforeOutputWasReceived(id);
        });
        clientInferiors[i].receiveOutputFromInferior();
    }
    serverTimer.expires_from_now(std::chrono::seconds(waitForOutputSeconds));
    serverTimer.async_wait([this](std::error_code ec){
        this->serverTimerExpiredBeforeOutputWasReceived();
    });
    serverInferior.receiveOutputFromInferior();
    currentServerStage = serverStages::START;
}

void GetAwayTest::outputReceivedFromInferior(std::string str){

}

void GetAwayTest::outputReceivedFromInferior(std::string str, int id){
    switch(currentClientStages[id]){
        case clientStages::START:
            if(str == gameStrings::clientHome){
                std::cout<<"Client " + std::to_string(id) + " Is Promoted To Home Current Stage"<<std::endl;
                currentClientStages[id] = clientStages::HOME;
            }else{
                std::cout<<"Could Not Promote The Client " + std::to_string(id) + " Because Of Unexpected String" << std::endl;
                std::cout<<"String Received is "<<str<<std::endl;
            }
            break;
        case clientStages::HOME:
            break;
        case clientStages::FIND_LOCAL_SERVERS:
            break;
        case clientStages::FIND_LOCAL_SERVERS_UPDATED:
            break;
        case clientStages::ENTER_YOUR_NAME_OR_USE_DEFAULT_TO_JOIN_SERVER:
            break;
        case clientStages::LOBBY:
            break;
        case clientStages::GAME_DORMANT:
            break;
        case clientStages::GAME_PERFORM_TURN:
            break;
    }
    if(str == gameStrings::clientHome){

    }
}

void GetAwayTest::serverTimerExpiredBeforeOutputWasReceived(){

}

void GetAwayTest::clientTimerExpiredBeforeOutputWasReceived(int id){

}
