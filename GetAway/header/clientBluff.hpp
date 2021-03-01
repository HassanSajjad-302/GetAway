#ifndef Bluff_CLIENTBLUFF_HPP
#define Bluff_CLIENTBLUFF_HPP


#include <string>
#include <vector>
#include <chrono>
#include "serverSession.hpp"
#include "messageTypeEnums.hpp"
#include "terminalInputBase.hpp"
#include "inputType.h"
#include "deckSuit.hpp"
#include "asio/io_context.hpp"



class clientLobby;

namespace Bluff {
    enum turnType{
        FIRST,
        PASS,
        NORMAL,
        CHECK
    };

    struct bluffTurn{
    public:
        turnType turn;
        deckSuit firstTurnSuit;
        int cardCount;
        std::vector<Card> checkTurn_LastTurnCards;

        bluffTurn(turnType firstTurn_, deckSuit firstTurnSuit_, int firstTurnCardCount_); //For first turn
        bluffTurn(turnType normalTurn_, int normalTurnCardCount_); //For Normal turn
        bluffTurn(turnType checkTurn_, std::vector<Card> checkTurn_LastTurnCards_); //For Check Turn
        explicit bluffTurn(turnType passTurn_); //For pass turn
    };

    class clientBluff: public terminalInputBase{
        class PF {
            //static inline std::string inputStatementBuffer;
            static inline std::string turnSequence;
            static inline std::string cardsString;
            static inline std::string waitingForTurn;
            static inline std::string turns;
            static inline void accumulateAndPrint();
        public:
            //input-statement-functions
            static void promptSimpleNonTurnInputAccumulate(bool clientOnly);

            static void promptFirstTurnOrNormalTurnSelectCards(const std::map<deckSuit, std::set<int>>& turnAbleCards_);

            static void setTurnSequence(const std::map<int, std::string>& gamePlayer_, const std::vector<int>& turnSequence_);

            static void
            setRoundTurns(const std::vector<std::tuple<int, bluffTurn>> &roundTurns,
                          const std::map<int, std::string> &gamePlayers);

            static void setRoundTurnsAccumulate(const std::vector<std::tuple<int, bluffTurn>> &roundTurns,
                                                const std::map<int, std::string> &gamePlayers);

            static void setWaitingForTurn(int waitingplayerId, const std::map<int, std::string>& gamePlayers);

            static void setCards(const std::map<deckSuit, std::set<int>>& cardsMap);

            static void promptFirstTurn(bool clientOnly);
            static void promptFirstTurnAccumulate(bool clientOnly);

            static void promptFirsTurnSelectDeckSuitAccumulate();

            static void promptNormalTurnAccumulate(bool clientOnly);
            static void promptAndInputWaitingForCardsAccumulate(bool clientOnly);
        };

        clientLobby& lobbyManager;
        const std::string& playerName;
        const std::map<int, std::string>& players;
        int myId = 0;
        inputType inputTypeExpected;

        void input(std::string inputString, inputType inputReceivedType) override;

        //Following Are Used For Game Management
        std::map<deckSuit, std::set<int>> myCards; //here cards are stored based on there 0-12 number and 0-4 enum value as in
        std::map<int, int> numberOfCards; //numberOfCards for each player
        std::vector<int> turnSequence;
        //Just Defined
        bool firstTurnOfRoundExpected;
        int numberOfCardsTurned;
        int passTurnCount;
        int cardsOnTableCount;
        int lastPlayerWhoTurnedId;
        int whoWillTurnNext_TurnSequenceIndex;
        std::vector<std::tuple<int, bluffTurn>> roundTurns;

        //Following Two Are Used For Helping The Cards Selection By User And In Sending Them.
        std::vector<int> selectedCardsIndicesFor_myCardsVectorLatest;
        std::vector<Card> myCardsVectorLatest;

        bool aPlayerIsMarkedForRemoval;
        int markedPlayerForRemovalId;
        deckSuit suitOfTheRound = static_cast<deckSuit>(-1);
        bool gameFinished = false;
        bool clientOnly;
    public:
        explicit
        clientBluff(clientLobby &lobbyManager_, const std::string& playerName_,
        const std::map<int, std::string>& players_, std::istream& in, int myId_, bool clientOnly_);

        void packetReceivedFromNetwork(std::istream &in, int receivedPacketSize);

        inline void setInputType(inputType inputType);

        inline void setBaseAndInputType(terminalInputBase* base_, inputType type);

        int nextInTurnSequence(int currentSessionId);

        void setBaseAndInputTypeFromclientChatMessage();

        void incrementWhoWillTurnNext_TurnSequenceIndex();

        bool inputHelperInCardsSelection(const std::string &inputHelper, inputType inputtype);

        void performFIRSTTurn();

        void performNORMALTurn();

        void performCHECKTurn(std::vector<Card> cardsReceived, bool didIWrongCheckedOrBluffed);

        void performPASSTurn();
    };
}

#endif //Bluff_CLIENTBLUFF_HPP
