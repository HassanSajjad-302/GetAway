#ifndef GETAWAY_SERVERLOBBYSESSIONSTATE_HPP
#define GETAWAY_SERVERLOBBYSESSIONSTATE_HPP

#include <memory>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <iostream>
class serverLobbySession;

//TODO
//All of these state classes should just have one instance of them.
//I will implement that later. It's constructor should be private.
//It should follow the singleton principle. So, it makes sure that
//it's instance is created when first serverLobbySession is created
//and destroyed when last serverLobbySession is destroyed.
// Represents the shared server state
class serverLobbySessionState
{
    // This simple method of tracking sessions only works with an implicit strand (i.e. a single-threaded server)
    std::unordered_set<serverLobbySession*> sessions;
    std::vector<std::string> gamePlayers;
    std::chrono::seconds time_per_turn = std::chrono::seconds(60);

    int classSendSize =0;

    friend std::ostream& operator<<(std::ostream& out, serverLobbySessionState& state){
        out << state.gamePlayers.size() << std::endl;
        for(auto & gamePlayer : state.gamePlayers){
            out << gamePlayer << std::endl;
        }
        out << state.time_per_turn.count() << std::endl;
        return out;
    }

    friend std::istream& operator>>(std::istream& in, serverLobbySessionState& state){
        int size;
        in >> size;
        in.ignore();
        for(int i=0;i<size;++i){
            char arr[61]; //This constant will be fed from somewhere else
            in.getline(arr,61);
            std::string str(arr);
            state.gamePlayers.push_back(str);
        }
        return in;
    }


public:
    explicit
    serverLobbySessionState();

    int getClassSendSize() const;
    void setClassSendSize(int size);
    void join  (serverLobbySession& session, const std::string& playerName);
    void leave (serverLobbySession& session);

    void stateSend  (std::string message, serverLobbySession* session);

};







In 2008 I provided a C++98 implementation of the Singleton design pattern that is lazy-evaluated, guaranteed-destruction, not-technically-thread-safe:
Can any one provide me a sample of Singleton in c++?

Here is an updated C++11 implementation of the Singleton design pattern that is lazy-evaluated, correctly-destroyed, and thread-safe.

class S
{
public:
    static S& getInstance()
    {
        static S    instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
private:
    S() {}                    // Constructor? (the {} brackets) are needed here.

    // C++ 03
    // ========
    // Don't forget to declare these two. You want to make sure they
    // are inaccessible(especially from outside), otherwise, you may accidentally get copies of
    // your singleton appearing.
    S(S const&);              // Don't Implement
    void operator=(S const&); // Don't implement

    // C++ 11
    // =======
    // We can use the better technique of deleting the methods
    // we don't want.
public:
    S(S const&)               = delete;
    void operator=(S const&)  = delete;

    // Note: Scott Meyers mentions in his Effective Modern
    //       C++ book, that deleted functions should generally
    //       be public as it results in better error messages
    //       due to the compilers behavior to check accessibility
    //       before deleted status
};



class Demo {
    static std::shared_ptr<Demo> d;
    Demo(){}
public:
    static std::shared_ptr<Demo> getInstance(){
        if(!d)
            d.reset(new Demo);
        return d;
    }
    ~Demo(){
        std::cout << "Object Destroyed " << std::endl;
    }

};
#endif //GETAWAY_SERVERLOBBYSESSIONSTATE_HPP
