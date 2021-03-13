#ifndef Bluff_CLIENTBLUFF_HPP
#define Bluff_CLIENTBLUFF_HPP


#include <string>
#include <vector>
#include <chrono>
#include "serverSession.hpp"
#include "messageTypeEnums.hpp"
#include "terminalInputBase.hpp"
#include "inputType.hpp"
#include "deckSuit.hpp"
#include "asio/io_context.hpp"



class clientLobby;

//client part is more sophisticated as it also involves keyboard input, screen output
//client part is divided in 2 cpp files. clientBluffPF.cpp has functions for printing while clientBluff.cpp has
//functions for printing and keyboard input.
//TODO: client related code is in namespace because there was name collision error due to turnType. Server code should
//also be in Bluff namespace. Other games in future will also be in their own namespace
namespace Bluff {

    //This enum is used by struct bluffTurn below:
    enum turnType{
        FIRST,
        PASS,
        NORMAL,
        CHECK
    };

    //This struct holds turn data which helps in printing roundTurns.
    //Different constructors are provided which initialize turn and one or other member
     struct bluffTurn{
    public:
        turnType turn;
        deckSuit firstTurnSuit;
        int cardCount;
        std::vector<Card> checkTurn_LastTurnCards;

        bluffTurn(turnType firstTurn_, deckSuit firstTurnSuit_, int firstTurnCardCount_); //For first turn. First Turn
        //will be displayed like e.g. "4 CLUB"
        bluffTurn(turnType normalTurn_, int normalTurnCardCount_); //For Normal turn. Normal Turn will be printed like
        //e.g. "3 MORE"
        bluffTurn(turnType checkTurn_, std::vector<Card> checkTurn_LastTurnCards_); //For Check Turn. Cards of last turn
        //will be displayed
        explicit bluffTurn(turnType passTurn_); //For pass turn. "PASS" will be displayed
    };

     //This class is core of client side logic. Important functions are input, packetReceivedFromNetwork and
     //setBaseAndInputTypeFromClientLobby
     //input is an override function which is called whenever user presses enter with the inputString
     //While packetReceivedFromNetwork is called by clientLobby when it receives packet of mtc::GAME type.
     //Another important function is setBaseAndInputTypeFromClientLobby which is called from clientLobby when
     //clientChat has sent message. It gives this, clientBluff, the opportunity to set itself as base in sati.cpp so
     //that it will receive next console input.

    class clientBluff: public terminalInputBase{
        //PF is for printing functions. This class helps in display. Some functions have Accumulate in end. These
        // functions call private function PF::accumulateAndPrint in the end. While others don't. This is done so that
        //accumulateAndPrint is called once whenever an event happens that alters any of following strings i.e.
        //turnSequence, cardsString, waitingForTurn or turns e.g. a packet is received or turn is performed by client.
        //Read comments in clientBluff class code for more details.
        class PF {
            static inline std::string turnSequence;
            static inline std::string cardsString;
            static inline std::string waitingForTurn;
            static inline std::string turns;

            static inline void accumulateAndPrint();
        public:
            static void promptSimpleNonTurnInputAccumulate(bool clientOnly);

            static void promptFirstTurnOrNormalTurnSelectCards(const std::map<deckSuit, std::set<int>>& turnAbleCards_);

            static void setTurnSequence(const std::map<int, std::string>& gamePlayer_, const std::vector<int>& turnSequence_);

            static void
            setRoundTurns(const std::vector<std::tuple<int, bluffTurn>> &roundTurns,
                          const std::map<int, std::string> &gamePlayers);

            static void setRoundTurnsAccumulate(const std::vector<std::tuple<int, bluffTurn>> &roundTurns,
                                                const std::map<int, std::string> &gamePlayers);

            static void setWaitingForTurn(int waitingPlayerId, const std::map<int, std::string>& gamePlayers);

            static void setCards(const std::map<deckSuit, std::set<int>>& cardsMap);

            static void promptFirstTurn(bool clientOnly);
            static void promptFirstTurnAccumulate(bool clientOnly);

            static void promptFirsTurnSelectDeckSuitAccumulate();

            static void promptNormalTurnAccumulate(bool clientOnly);
            static void promptAndInputWaitingForCardsAccumulate(bool clientOnly);
        };

        clientLobby& lobby;
        const std::string& playerName;
        const std::map<int, std::string>& players;
        int myId = 0;
        inputType inputTypeExpected;

        //This function is called by io.run() loop in main.cpp when this is posted by sati.cpp on asio::io_context io.
        //inputString is the input, that user entered on console screen while inputReceivedType
        //determines how input processes the inputString and how game continues.
        void input(std::string inputString, inputType inputReceivedType) override;

        //Following Are Used For Game Management
        std::map<deckSuit, std::set<int>> myCards;
        std::map<int, int> numberOfCards; //numberOfCards of each player
        std::vector<int> turnSequence;
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
        bool clientOnly;
    public:
        //It accepts clientOnly_. If this class is used from home.cpp, then clientOnly_ is passed true. If used from
        //serverListener then clientOnly_ is passed false as our application is not client only. It is also the server.
        //This information is used to display differently depending on clientOnly_ value.
        explicit
        clientBluff(clientLobby &lobby_, const std::string& playerName_,
                    const std::map<int, std::string>& players_, std::istream& in, int myId_, bool clientOnly_);

        void packetReceivedFromNetwork(std::istream &in, int receivedPacketSize);

        inline void setInputType(inputType inputType);

        inline void setBaseAndInputType(terminalInputBase* base_, inputType type);

        int nextInTurnSequence(int currentSessionId);

        void setBaseAndInputTypeFromClientLobby();

        void incrementWhoWillTurnNext_TurnSequenceIndex();

        bool inputHelperInCardsSelection(const std::string &inputHelper, inputType inputtype);

        void performFIRSTTurn();

        void performNORMALTurn();

        void performCHECKTurn(std::vector<Card> cardsReceived, bool didIWrongCheckedOrBluffed);

        void performPASSTurn();
    };
}

#endif //Bluff_CLIENTBLUFF_HPP
