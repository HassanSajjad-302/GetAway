#include <random>
#include <serverGetAway.hpp>
#include <utility>
#include <cassert>
#include "constants.h"
#include "resourceStrings.hpp"
#include "serverLobby.hpp"
#include "sati.hpp"
#include "clientBluff.hpp"

serverGetAway::
serverGetAway(const std::map<int, std::tuple<std::string,
        std::unique_ptr<serverSession<serverLobby>>>>& gameData_, serverLobby& lobbyManager_):
        players{gameData_}, lobbyManager{lobbyManager_}{
    initializeGame();
    doFirstTurnOfFirstRound();
    firstRound = true;
}

void serverGetAway::packetReceivedFromNetwork(std::istream &in, int receivedPacketSize, int sessionId){
    mtgg messageTypeReceived;
    //STEP 1;
    in.read(reinterpret_cast<char*>(&messageTypeReceived), sizeof(messageTypeReceived));
    if(messageTypeReceived == mtgg::GAMETURNCLIENT){
        constants::Log("GAMETURNCLIENT received");
        int index;
        if(indexGamePlayerDataFromId(sessionId, index)){
            if(gamePlayersData[index].turnExpected) {
                constants::Log("Turn Expected from this GameTurnClient");
                deckSuit suit;
                int receivedCardNumber;
                in.read(reinterpret_cast<char *>(&suit), sizeof(suit));
                in.read(reinterpret_cast<char *>(&receivedCardNumber), sizeof(receivedCardNumber));
                managementGAMETURNCLIENTReceived(sessionId, Card(suit, receivedCardNumber));
            }else{
                constants::Log("Turn Not Expected From This Client");
            }
        }else{
            resourceStrings::print("getAwayPData with this id could not be found in the gamePlayersData. "
                                   "Most likely it was removed because it's game ended. But still receieved"
                                   "a GAMETURNCLIENT message from it\r\n");
        }
    }
    else{
        resourceStrings::print("Unexpected Packet Type Received in class serverLobbyManager\r\n");
    }
}

void serverGetAway::sendGAMETURNSERVERTOAllExceptOne(int sessionId, Card card) {
    constants::Log("Sending GameTurnServerToAllExceptOne to all. Called By Session-Id {}", sessionId);
    constants::Log("Sending Card {} {}", deckSuitValue::displaySuitType[(int) card.suit],
                 deckSuitValue::displayCards[card.cardNumber]);
    for(auto& player: players){
        if(player.first != sessionId){
            auto& playerSession = std::get<1>(player.second);
            std::ostream& out = playerSession->out;

            //STEP 1;
            out.write(reinterpret_cast<const char*>(&constants::mtcGame), sizeof(constants::mtcGame));
            mtgg t = mtgg::GAMETURNSERVER;
            out.write(reinterpret_cast<char*>(&t), sizeof(t));
            //STEP 2;
            out.write(reinterpret_cast<char *>(&sessionId), sizeof(sessionId));
            //STEP 3;
            out.write(reinterpret_cast<char *>(&card.suit), sizeof(card.suit));
            //STEP 4;
            out.write(reinterpret_cast<char*>(&card.cardNumber), sizeof(card.cardNumber));

            playerSession->sendMessage();
        }
    }
}

void serverGetAway::initializeGame(){
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
        auto p = getAwayPData(playersIdList[i]);
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

void serverGetAway::doFirstTurnOfFirstRound(){
    //check for auto first turn possibilities
    constants::initializeCards(flushedCards);
    for(auto& player: gamePlayersData){
        player.turnExpected = true;
        if(player.cards.find(deckSuit::SPADE)->second.empty()){
            player.turnTypeExpected = turnType::FIRSTROUNDANY;
        }else{
            player.turnTypeExpected = turnType::FIRSTROUNDSPADE;
        }
    }

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
void serverGetAway::checkForCardsCount(){
    int cardsCount = 0;
    cardsCount += constants::cardsCount(flushedCards);
    cardsCount += roundTurns.size();
    for(auto& p: gamePlayersData){
        cardsCount += constants::cardsCount(p.cards);
    }
    if(cardsCount != constants::DECKSIZE){
        resourceStrings::print("Flushed Cards " + std::to_string(constants::cardsCount(flushedCards)) + "\r\n");
        resourceStrings::print("Round Turns " + std::to_string(roundTurns.size()) + "\r\n");
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

void serverGetAway::managementGAMETURNCLIENTReceived(int sessionId, Card cardReceived) {

    CHECKCARDCOUNT
    constants::Log("Game Turn Client Received From{}", std::get<0>(players.find(sessionId)->second));
    constants::Log("Card Received Is {} {}", deckSuitValue::displaySuitType[(int)cardReceived.suit],
                 deckSuitValue::displayCards[cardReceived.cardNumber]);
    //iterating over gamePlayerData
    auto turnReceivedPlayer = std::find_if(gamePlayersData.begin(), gamePlayersData.end(),
                                               [sessionId](getAwayPData& p){
        return p.id == sessionId;
    });//Be Careful Of turnReceivedPlayer Invalidation during the function
    assert(turnReceivedPlayer != gamePlayersData.end() && "no gamePlayerData id matches with sessionId");
    auto iterSuit = turnReceivedPlayer->cards.find(cardReceived.suit);
    if(iterSuit == turnReceivedPlayer->cards.end()) {
        resourceStrings::print("A Client Turn Received But Not Acted Upon. Card Number not in card list user.\r\n");
        return;
    }
    auto iter = iterSuit->second.find(cardReceived.cardNumber);
    if(iter == iterSuit->second.end()) {
        resourceStrings::print("A Client Turn Received But Not Acted Upon. Card Number not in card list user.\r\n");
        return;
    }
    if(firstRound){
        if(turnReceivedPlayer->turnTypeExpected == turnType::FIRSTROUNDANY){
            constants::Log("First Round Any was expected from this user");
            doTurnReceivedOfFirstRound(turnReceivedPlayer, cardReceived);
        }else{
            if(cardReceived.suit != deckSuit::SPADE) {
                resourceStrings::print("A Turn Received But Not Acted Upon. In First Turn Client"
                                       "Turned Other Card When It Had The Spade\r\n");
                return;
            }
            constants::Log("First Round SPADE was expected from this user");
            doTurnReceivedOfFirstRound(turnReceivedPlayer, cardReceived);//paste that code after this line
        }

    }else{
        Turn(turnReceivedPlayer, cardReceived);
    }
}

void
serverGetAway::doTurnReceivedOfFirstRound(
        std::vector<getAwayPData>::iterator turnReceivedPlayer, Card cardReceived) {
    turnCardNumberOfGamePlayerIterator(turnReceivedPlayer, cardReceived);
    roundTurns.emplace_back(turnReceivedPlayer->id, cardReceived);
    if(roundTurns.size() != gamePlayersData.size()) {
        return;
    }
    firstRound = false;
    for(auto& rt: roundTurns){
        flushedCards.find(std::get<1>(rt).suit)->second.emplace(std::get<1>(rt).cardNumber);
    }
    for(auto t: roundTurns){
        if(std::get<1>(t).suit == deckSuit::SPADE && std::get<1>(t).cardNumber == 0){
            //This is ace of spade
            for(auto it = gamePlayersData.begin(); it!=gamePlayersData.end(); ++it){
                if(it->id == std::get<0>(t)){
                    roundTurns.clear();
                    newRoundTurn(it);
                    break;
                }
            }
            break;
        }
    }
}

void serverGetAway::newRoundTurn(std::vector<getAwayPData>::iterator currentGamePlayer){
        currentGamePlayer->turnTypeExpected = turnType::ROUNDFIRSTTURN;
        currentGamePlayer->turnExpected = true;
}

void serverGetAway::Turn(std::vector<getAwayPData>::iterator currentTurnPlayer, Card card){
    if(currentTurnPlayer->turnTypeExpected == turnType::ROUNDFIRSTTURN){ //First Turn Of Round Was Expected
        performFirstOrMiddleTurn(currentTurnPlayer, card, true);
    }else if (currentTurnPlayer->turnTypeExpected == turnType::ROUNDMIDDLETURN){
        performFirstOrMiddleTurn(currentTurnPlayer, card, false);
    }else if(currentTurnPlayer->turnTypeExpected == turnType::ROUNDLASTTURN){
        performLastOrThullaTurn(currentTurnPlayer, card, true);
    }else if(currentTurnPlayer->turnTypeExpected == turnType::THULLA){//A Thulla Was Expected
        performLastOrThullaTurn(currentTurnPlayer, card, false);
    }
}

void serverGetAway::performFirstOrMiddleTurn(
        std::vector<getAwayPData>::iterator currentTurnPlayer, Card card, bool firstTurn){

    constants::Log("Perform First Or Middle Turn Called. firstTurn bool value is {}", std::to_string(firstTurn));

    if(firstTurn){
        assert(roundTurns.empty() && "If it is first turn then roundTurns should be empty");
        constants::Log("First Turn Of The Round Is Received. Suit Set For The Round {}",
                     deckSuitValue::displaySuitType[(int) card.suit]);
        //First Turn Was Expected
        suitOfTheRound = card.suit;
    }else{
        assert(!roundTurns.empty() && "If it is not first turn then roundTurns should not be empty");
        if(card.suit != suitOfTheRound){
            resourceStrings::print("A Turn Received But Not Acted Upon Because"
                                   " Card is not of the required suit\r\n");
        }
    }

    turnCardNumberOfGamePlayerIterator(currentTurnPlayer, card);
    roundTurns.emplace_back(currentTurnPlayer->id, card);
    currentTurnPlayer->cards.find(card.suit)->second.erase(card.cardNumber);

    std::vector<getAwayPData>::iterator nextGamePlayerIterator;
    nextGamePlayerIterator = std::next(currentTurnPlayer);
    if(nextGamePlayerIterator == gamePlayersData.end()){
        nextGamePlayerIterator = gamePlayersData.begin();
    }
    assert(constants::cardsCount(nextGamePlayerIterator->cards) > 0 && "Card Count should be greater than one");


    if(nextGamePlayerIterator->cards.find(suitOfTheRound)->second.empty()) {
        nextGamePlayerIterator->turnTypeExpected = turnType::THULLA;
        nextGamePlayerIterator->turnExpected = true;
        constants::Log("nextGamePlayerIterator Thulla Expected. Id {}", nextGamePlayerIterator->id);
    }else{
        if(roundTurns.size() == gamePlayersData.size() -1){
            constants::Log("nextGamePlayerIterator ROUNDLASTTURN Expected. Id {}", nextGamePlayerIterator->id);
            nextGamePlayerIterator->turnTypeExpected = turnType::ROUNDLASTTURN;
        }else{
            constants::Log("nextGamePlayerIterator ROUNDMIDDLETURN Expected. Id {}", nextGamePlayerIterator->id);
            nextGamePlayerIterator->turnTypeExpected = turnType::ROUNDMIDDLETURN;
        }
        constants::Log("Waiting For Turn. Turn Expected Called");
        nextGamePlayerIterator->turnExpected = true;
    }
    CHECKCARDCOUNT
}

void serverGetAway::performLastOrThullaTurn(
        std::vector<getAwayPData>::iterator currentTurnPlayer, Card card, bool lastTurn){
    constants::Log("Perform First Or Middle Turn Called. firstTurn bool value is {}", std::to_string(lastTurn));
    std::vector<getAwayPData>::iterator roundKing;
    if(lastTurn){
        if(card.suit != suitOfTheRound){
            resourceStrings::print("A Turn Received But Not Acted Upon"
                                   " Because Card is not of the required suit\r\n");
            return;
        }
        roundTurns.emplace_back(currentTurnPlayer->id, card);
        roundKing = roundKingGamePlayerDataIterator();
        for(auto cn: roundTurns){
            flushedCards.find(std::get<1>(cn).suit)->second.emplace(std::get<1>(cn).cardNumber);
        }
    }else{
        roundKing = roundKingGamePlayerDataIterator();
        roundTurns.emplace_back(currentTurnPlayer->id, card);
        for(auto cn: roundTurns){
            roundKing->cards.find(std::get<1>(cn).suit)->second.emplace(std::get<1>(cn).cardNumber);
        }
    }
    turnCardNumberOfGamePlayerIterator(currentTurnPlayer, card);

    gamePlayersData.erase(std::remove_if(gamePlayersData.begin(), gamePlayersData.end(),
                                         [](getAwayPData& s){
                                             return constants::cardsCount(s.cards) == 0;
                                         }), gamePlayersData.end());

    roundTurns.clear();
    if(gamePlayersData.empty()){
        //match drawn
        lobbyManager.gameExitFinished();
        return;
    }
    if(gamePlayersData.size() == 1){
        //That id left player has lost
        lobbyManager.gameExitFinished();
        return;
    }
    newRoundTurn(roundKing);
    CHECKCARDCOUNT
}

void serverGetAway::turnCardNumberOfGamePlayerIterator(std::vector<getAwayPData>::iterator turnReceivedPlayer,
                                                       Card card){

    sendGAMETURNSERVERTOAllExceptOne(turnReceivedPlayer->id, card);
    turnReceivedPlayer->cards.find(card.suit)->second.erase(card.cardNumber);
    turnReceivedPlayer->turnExpected = false;
}

std::vector<getAwayPData>::iterator serverGetAway::roundKingGamePlayerDataIterator(){
    int highestCardHolderId = std::get<0>(*roundTurns.begin());
    int highestCardNumber = std::get<1>(*roundTurns.begin()).cardNumber;
    for(auto turns: roundTurns){
        int cardNumber = std::get<1>(turns).cardNumber;
        assert((cardNumber >= 0 && cardNumber < constants::SUITSIZE) && "Card-Number not in range\r\n");
        if(cardNumber == 0 ){
            highestCardHolderId = std::get<0>(turns);
            break;
        }else{
            if(cardNumber > highestCardNumber){
                highestCardNumber = cardNumber;
                highestCardHolderId = std::get<0>(turns);
            }
        }
    }
    for(auto it = gamePlayersData.begin(); it != gamePlayersData.end(); ++it){
        if(it->id == highestCardHolderId){
            return it;
        }
    }
    throw(std::logic_error("HighestCardHolder Id not present in gamePlayerData\r\n"));
}

bool serverGetAway::indexGamePlayerDataFromId(int id, int& index){
    for(u_int i=0; i < gamePlayersData.size(); ++i){
        if(gamePlayersData[i].id == id){
            index = i;
            return true;
        }
    }
    return false;
}