set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(srcFiles "main.cpp" "../GetAway/src/androidClientMain.cpp" "../GetAway/src/resourceStrings.cpp"
        "../GetAway/src/clientChat.cpp" "../GetAway/src/androidServerMain.cpp" "../GetAway/src/deckSuit.cpp"
        "../GetAway/src/serverChat.cpp" "../GetAway/src/serverGetAway.cpp"
        "../GetAway/src/sati.cpp" "../GetAway/src/serverLobby.cpp" "../GetAway/src/serverListener.cpp"
        "../GetAway/src/clientLobbyPF.cpp" "../GetAway/src/satiAndroid.cpp"
        "../GetAway/src/clientGetAwayPF.cpp" "../GetAway/src/home.cpp" "../GetAway/src/homePF.cpp"
        "../GetAway/src/serverLobbyPF.cpp" "../GetAway/src/getAwayPData.cpp" "../GetAway/src/clientChatPF.cpp"
        "../GetAway/src/replacementStreamBuff.cpp" "../GetAway/src/clientLobby.cpp"
        "../GetAway/src/serverListenerPF.cpp" "../GetAway/src/clientGetAway.cpp"
        "../GetAway/src/constants.cpp" "../GetAway/src/serverBluff.cpp" "../GetAway/src/clientBluff.cpp"
        "../GetAway/src/clientBluffPF.cpp" "../GetAway/src/bluffPData.cpp")

add_executable(ClientTest ${srcFiles})
target_compile_definitions(ClientTest PUBLIC ASIO_STANDALONE)

find_package(spdlog REQUIRED)
target_link_libraries(ClientTest PUBLIC spdlog::spdlog)

find_package(Threads REQUIRED)
target_link_libraries(ClientTest PRIVATE Threads::Threads)

target_include_directories(ClientTest PUBLIC ../GetAway/header)
target_include_directories(ClientTest PUBLIC ../asio/asio/include)
