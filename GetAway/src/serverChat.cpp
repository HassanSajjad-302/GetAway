
#include "serverChat.hpp"
#include "serverLobby.hpp"

serverChat::serverChat(
        const std::map<int, std::tuple<std::string, std::unique_ptr<session<serverLobby, true>>>> &players_):
        players{players_}{

}

void serverChat::packetReceivedFromNetwork(std::istream &in, int receivedPacketSize, int sessionId){
    int senderId;
    //TODO
    //This is an extra read, delete it.
    //STEP 2;
    in.read(reinterpret_cast<char*>(&senderId), sizeof(senderId));
    assert(senderId == sessionId);
#if defined(_WIN32) || defined(_WIN64)
    char* arr = new char[receivedPacketSize - 4];
#endif
#ifdef __linux__
    char arr[receivedPacketSize - 4];
#endif
    //STEP 3;
    in.getline(arr, receivedPacketSize -4);
    CHATMESSAGEReceived(std::string(arr), sessionId);
#if defined(_WIN32) || defined(_WIN64)
    delete[] arr;
#endif

}

void serverChat::CHATMESSAGEReceived(const std::string &chatMessageReceived, int excitedSessionId) {
    for(auto& player: players){
        if(player.first != excitedSessionId){
            auto& playerSession = std::get<1>(player.second);
            std::ostream& out = playerSession->out;

            //STEP 1;
            out.write(reinterpret_cast<const char*>(&constants::mtcMessage), sizeof(constants::mtcMessage));
            //STEP 2;
            out.write(reinterpret_cast<char *>(&excitedSessionId), sizeof(excitedSessionId));
            //STEP 3;
            out << chatMessageReceived << std::endl;

            playerSession->sendMessage();
        }
    }
    resourceStrings::print("Message Sent To All Clients\r\n");
}
