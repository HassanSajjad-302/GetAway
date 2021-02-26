
#include <random>
#include <serverBluff.hpp>
#include <utility>
#include <cassert>
#include "constants.h"
#include "resourceStrings.hpp"
#include "serverLobby.hpp"
#include "sati.hpp"

serverBluff::
serverBluff(const std::map<int, std::tuple<std::string,
        std::unique_ptr<serverSession<serverLobby>>>>& gameData_, serverLobby& lobbyManager_):
        players{gameData_}, lobbyManager{lobbyManager_}{
    initializeGame();
    doFirstTurnOfFirstRound();
    firstTurnOfRoundExpected = true;
    whoWillTurnNext_GamePlayersDataIndex = 0;
    passTurnCount = 0;
}

void serverBluff::incrementWhoWillTurnNext_GamePlayersDataIndex(){
    ++whoWillTurnNext_GamePlayersDataIndex;
    if(whoWillTurnNext_GamePlayersDataIndex == gamePlayersData.size()){
        whoWillTurnNext_GamePlayersDataIndex = 0;
    }
}

void serverBluff::packetReceivedFromNetwork(std::istream &in, int receivedPacketSize, int sessionId){

    assert(sessionId == gamePlayersData[whoWillTurnNext_GamePlayersDataIndex].id &&
    "Packet received from player whose id not equal to gamePlayersData[whoWillTurnNext_GamePlayersDataIndex].id Most"
    "Probably whoWillTurnNext_GamePlayersDataIndex was not assigned right value on last turn");
    if(firstTurnOfRoundExpected){
        in.read(reinterpret_cast<char *>(&suitOfTheRound), sizeof(suitOfTheRound));
        in.read(reinterpret_cast<char *>(&numberOfCardsTurned), sizeof(numberOfCardsTurned));
        cardsOnTable.clear();
        Card card;
        for(int i=0; i<numberOfCardsTurned; ++i){
            in.read(reinterpret_cast<char *>(&card), sizeof (card));
            gamePlayersData[whoWillTurnNext_GamePlayersDataIndex].removeCard(card);
            cardsOnTable.emplace_back(card);
        }
        firstTurnOfRoundExpected = false;
        lastPlayerWhoTurnedId = sessionId;
        passTurnCount = 0;

        if(constants::cardsCount(gamePlayersData[whoWillTurnNext_GamePlayersDataIndex].cards) == 0){
            aPlayerIsMarkedForRemoval = true;
            markedPlayerForRemovalId = gamePlayersData[whoWillTurnNext_GamePlayersDataIndex].id;
        }
        incrementWhoWillTurnNext_GamePlayersDataIndex();

        //Send this turn to everyone except the one from whom we received.
        for(auto& p: players){
            if(p.first != sessionId){
                auto& out = std::get<1>(p.second)->out;
                out.write(reinterpret_cast<const char*>(&constants::mtcGame), sizeof(constants::mtcGame));
                //STEP 1;
                out.write(reinterpret_cast<char*>(&suitOfTheRound), sizeof(suitOfTheRound));
                out.write(reinterpret_cast<char *>(&numberOfCardsTurned), sizeof(numberOfCardsTurned));
                for(auto c: cardsOnTable){
                    out.write(reinterpret_cast<char *>(&c), sizeof (c));
                }
                std::get<1>(p.second)->sendMessage();
            }
        }
    }else{
        mtgb messageTypeReceived;
        in.read(reinterpret_cast<char *>(&messageTypeReceived), sizeof(messageType));
        switch (messageTypeReceived) {
            case mtgb::CLIENT_TURNNORMAL:{
                in.read(reinterpret_cast<char *>(&numberOfCardsTurned), sizeof(numberOfCardsTurned));
                Card card;
                for(int i=0; i<numberOfCardsTurned; ++i){
                    in.read(reinterpret_cast<char *>(&card), sizeof (card));
                    gamePlayersData[whoWillTurnNext_GamePlayersDataIndex].removeCard(card);
                    cardsOnTable.emplace_back(card);
                }
                passTurnCount = 0;
                lastPlayerWhoTurnedId = sessionId;

                if(aPlayerIsMarkedForRemoval){
                    aPlayerIsMarkedForRemoval = false;
                    int index = 0;
                    for(int i=0;i<gamePlayersData.size(); ++i){
                        if(gamePlayersData[i].id == markedPlayerForRemovalId){
                            index = i;
                            break;
                        }
                    }
                    gamePlayersData.erase(gamePlayersData.begin(), gamePlayersData.begin() + index);
                }
                if(constants::cardsCount(gamePlayersData[whoWillTurnNext_GamePlayersDataIndex].cards) == 0){

                    aPlayerIsMarkedForRemoval = true;
                    markedPlayerForRemovalId = sessionId;
                }
                incrementWhoWillTurnNext_GamePlayersDataIndex();

                //Send this turn to everyone except the one from whom we received.
                for(auto& p: players){
                    if(p.first != sessionId){
                        auto& out = std::get<1>(p.second)->out;
                        out.write(reinterpret_cast<const char*>(&constants::mtcGame), sizeof(constants::mtcGame));
                        //STEP 1;
                        mtgb turnType = mtgb::SERVER_CLIENTTURNNORMAL;
                        out.write(reinterpret_cast<char *>(&turnType), sizeof(turnType));
                        out.write(reinterpret_cast<char *>(&numberOfCardsTurned), sizeof(numberOfCardsTurned));

                        std::get<1>(p.second)->sendMessage();
                    }
                }
                break;
            }
            case mtgb::CLIENT_TURNCHECK:{

                firstTurnOfRoundExpected = true;
                passTurnCount = 0;

                bool bluff = false;
                for(std::size_t i=cardsOnTable.size() - numberOfCardsTurned; i<cardsOnTable.size(); ++i){
                    if(cardsOnTable[i].suit != suitOfTheRound){
                        bluff = true;
                        break;
                    }
                }

                if(bluff) {
                    if (aPlayerIsMarkedForRemoval) {
                        aPlayerIsMarkedForRemoval = false;
                        assert(aPlayerIsMarkedForRemoval == lastPlayerWhoTurnedId
                               &&
                               "If a player id is marked for removal, then it's id should be equal to the lastplayerwhoturned "
                               "id. Otherwise it suggests that aPlayerIsMarkedForRemoval was not falsed after someone else "
                               "played cards after setting it to true which breaks the invariant.");
                    }

                    bool success = indexGamePlayerDataFromId(lastPlayerWhoTurnedId,
                                                             whoWillTurnNext_GamePlayersDataIndex);
                    assert(success == true && "Could not find the lastplayerturnid in gamePlayersData");
                }
                for(auto & i : cardsOnTable){
                    gamePlayersData[whoWillTurnNext_GamePlayersDataIndex].insertCard(i);
                }
                cardsOnTable.clear();

                for(auto& p: players){
                    auto& out = std::get<1>(p.second)->out;
                    out.write(reinterpret_cast<const char*>(&constants::mtcGame), sizeof(constants::mtcGame));
                    //STEP 1;
                    mtgb turnType = mtgb::SERVER_CLIENTTURNCHECK;
                    out.write(reinterpret_cast<char *>(&turnType), sizeof(turnType));
                    for(std::size_t i=cardsOnTable.size() - numberOfCardsTurned; i<cardsOnTable.size(); ++i){
                        out.write(reinterpret_cast<char*>(&cardsOnTable[i]), sizeof(cardsOnTable[i]));
                    }
                    std::get<1>(p.second)->sendMessage();
                }
            }
            case mtgb::CLIENT_TURNPASS:{
                ++passTurnCount;
                if(aPlayerIsMarkedForRemoval){
                    if(passTurnCount == gamePlayersData.size() -1){
                        aPlayerIsMarkedForRemoval = false;
                        firstTurnOfRoundExpected = true;
                        for(auto & i : cardsOnTable){
                            flushedCards.find(i.suit)->second.emplace(i.cardNumber);
                        }
                        cardsOnTable.clear();
                        incrementWhoWillTurnNext_GamePlayersDataIndex();
                        gamePlayersData.erase(gamePlayersData.begin(),
                                              gamePlayersData.begin() + whoWillTurnNext_GamePlayersDataIndex);
                        incrementWhoWillTurnNext_GamePlayersDataIndex();
                    }
                }else if(passTurnCount == gamePlayersData.size()){
                    firstTurnOfRoundExpected = true;
                    for(auto & i : cardsOnTable){
                        flushedCards.find(i.suit)->second.emplace(i.cardNumber);
                    }
                    cardsOnTable.clear();
                    incrementWhoWillTurnNext_GamePlayersDataIndex();
                }

                for(auto& p: players){
                    if(p.first != sessionId){
                        auto& out = std::get<1>(p.second)->out;
                        out.write(reinterpret_cast<const char*>(&constants::mtcGame), sizeof(constants::mtcGame));
                        //STEP 1;
                        mtgb turnType = mtgb::SERVER_CLIENTTURNPASS;
                        out.write(reinterpret_cast<char *>(&turnType), sizeof(turnType));
                        std::get<1>(p.second)->sendMessage();
                    }
                }
            }
        }
    }
}

void serverBluff::initializeGame(){
    std::random_device rd{};
    //todo
    //providing a same value to random engine for testing
    auto rng = std::default_random_engine { rd() };

    int numberOfPlayersWithExtraCards = constants::DECKSIZE % players.size();

    std::vector<int> playersIdList;
    for(const auto& player: players){
        playersIdList.push_back(player.first);
    }
    std::shuffle(playersIdList.begin(), playersIdList.end(), rng);

    std::vector<int> playerWithExtraCardIdsList;
    playerWithExtraCardIdsList.reserve(numberOfPlayersWithExtraCards);
    for(int i=0;i<numberOfPlayersWithExtraCards;++i){
        playerWithExtraCardIdsList.push_back(playersIdList[i]);
    }
    std::shuffle(playerWithExtraCardIdsList.begin(), playerWithExtraCardIdsList.end(), rng);

    int cards[constants::DECKSIZE];
    for(int i=0;i<constants::DECKSIZE;++i){
        cards[i] = i;
    }
    std::shuffle(cards, cards + constants::DECKSIZE, rng);

    int normalNumberOfCards = constants::DECKSIZE / players.size();

    int cardsCount = 0;
    for(u_int i=0; i<playersIdList.size(); ++i){
        auto p = bluffPData(playersIdList[i]);
        if(std::find(playerWithExtraCardIdsList.begin(), playerWithExtraCardIdsList.end(), players.find(i)->first) !=
           playerWithExtraCardIdsList.end()){
            for(int j=0; j<normalNumberOfCards+1; ++j){
                assert(cards[cardsCount] >= 0 && cards[cardsCount] < constants::DECKSIZE && "Card Id is out-of-range");
                auto suit = static_cast<deckSuit>(cards[cardsCount] / constants::SUITSIZE);
                int cardNumber = cards[cardsCount] % constants::SUITSIZE;
                p.insertCard(Card(suit, cardNumber));
                ++cardsCount;
            }
        }else{
            for(int j =0; j<normalNumberOfCards; ++j){
                assert(cards[cardsCount] >= 0 && cards[cardsCount] < constants::DECKSIZE && "Card Id is out-of-range");
                auto suit = static_cast<deckSuit>(cards[cardsCount] / constants::SUITSIZE);
                int cardNumber = cards[cardsCount] % constants::SUITSIZE;
                p.insertCard(Card(suit, cardNumber));
                ++cardsCount;
            }
        }
        gamePlayersData.emplace_back(std::move(p));
    }

#ifndef NDEBUG
    int size = 0;
    for(auto& p: gamePlayersData){
        size += constants::cardsCount(p.cards);
    }
    assert(size == constants::DECKSIZE && "Sum Of Cards distributed not equal to constants::DECKSIZE");
#endif

}

void serverBluff::doFirstTurnOfFirstRound(){
    //check for auto first turn possibilities
    constants::initializeCards(flushedCards);

    //first turn message sending
    for(u_int i=0; i<gamePlayersData.size(); ++i){
        auto& player = gamePlayersData[i];
        int currentIndexGamePlayersData = i;

        auto& playerSession = std::get<1>(players.find(gamePlayersData[i].id)->second);
        std::ostream& out = playerSession->out;

        //STEP 1;
        out.write(reinterpret_cast<const char*>(&constants::mtcGame), sizeof(constants::mtcGame));
        auto &p = gamePlayersData[currentIndexGamePlayersData];
        int handSize = constants::cardsCount(p.cards);
        //STEP 2;
        out.write(reinterpret_cast<char*>(&handSize),sizeof(handSize));
        for(auto& cardPair: p.cards){
            for(auto c: cardPair.second) {
                //STEP 3; //deckSuit
                out.write(reinterpret_cast<const char *>(&cardPair.first), sizeof(cardPair.first));
                //STEP 4;
                out.write(reinterpret_cast<char *>(&c), sizeof(c));
            }
        }

        //STAGE 2;
        for(auto& turnSequenceId: gamePlayersData){
            int sequenceId = turnSequenceId.id;
            //STEP 5; //Turn Sequence
            out.write(reinterpret_cast<char*>(&sequenceId), sizeof(sequenceId));
        }

        for(auto& gp: gamePlayersData){
            int gpId = gp.id;
            //STEP 10;
            out.write(reinterpret_cast<char*>(&gpId), sizeof(gpId));
            int gpCardsSize = constants::cardsCount(gp.cards);
            //STEP 11;
            out.write(reinterpret_cast<char*>(&gpCardsSize), sizeof(gpCardsSize));
        }
        playerSession->sendMessage();
    }
}

#ifndef NDEBUG
void serverBluff::checkForCardsCount(){
    int cardsCount = 0;
    cardsCount += constants::cardsCount(flushedCards);
    cardsCount += cardsOnTable.size();
    for(auto& p: gamePlayersData){
        cardsCount += constants::cardsCount(p.cards);
    }
    if(cardsCount != constants::DECKSIZE){
        resourceStrings::print("Flushed Cards " + std::to_string(constants::cardsCount(flushedCards)) + "\r\n");
        resourceStrings::print("Round Turns " + std::to_string(cardsOnTable.size()) + "\r\n");
        for(auto& p: gamePlayersData){
            resourceStrings::print("Player Id " + std::to_string(p.id) + "  Player Cards Size " +
                                   std::to_string(constants::cardsCount(p.cards)) + "\r\n");
        }
    }
    assert(cardsCount == constants::DECKSIZE && "Card-Count not equal to 52 error");

}
#define CHECKCARDCOUNT checkForCardsCount();
#else
#define CHECKCARDCOUNT
#endif

bool serverBluff::indexGamePlayerDataFromId(int id, int& index){
    for(u_int i=0; i < gamePlayersData.size(); ++i){
        if(gamePlayersData[i].id == id){
            index = i;
            return true;
        }
    }
    return false;
}
