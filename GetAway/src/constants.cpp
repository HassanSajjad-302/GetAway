#include "constants.h"
#include "sati.hpp"
#include "resourceStrings.hpp"
#include <cassert>
int constants::cardsCount(const std::map<deckSuit, std::set<int>> &cards) {
    int count = 0;

    for(auto & cardPair: cards){
        count += cardPair.second.size();
    }
    assert(count < DECKSIZE && "cardSize has been passed more than DECKSIZE");
    return count;
}

void constants::initializeCards(std::map<deckSuit, std::set<int>>& cards){
    cards.clear();
    std::set<int> s;
    for(int i=0;i<4;++i){
        cards.emplace(static_cast<deckSuit>(i), s);
    }
}

bool constants::inputHelper(const std::string& inputString, int lower, int upper, inputType notInRange_,
                             inputType invalidInput_, int& input_){
    try{
        int num = std::stoi(inputString);
        if(num>=lower && num<=upper){
            input_ = num;
            return true;
        }else{
            sati::getInstance()->accumulatePrint();
            sati::getInstance()->setInputType(notInRange_);
            resourceStrings::print("Please enter integer in range\r\n");
            return false;
        }
    }
    catch (std::invalid_argument& e) {
        sati::getInstance()->accumulatePrint();
        sati::getInstance()->setInputType(invalidInput_);
        resourceStrings::print("Invalid Input\r\n");
        return false;
    }
}
