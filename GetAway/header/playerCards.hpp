//
// Created by hassan on 11/13/20.
//

#ifndef GETAWAY_PLAYERCARDS_HPP
#define GETAWAY_PLAYERCARDS_HPP

#include<list>

class playerCards{
private:
    std::list<int> cards;
public:
    playerCards();
    void addCard(int Id);
    void removeCard(int Id);
    int getCardCount();
    const std::list<int>& getCards();
};

#endif //GETAWAY_PLAYERCARDS_HPP
