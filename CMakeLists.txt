get_directory_property(hasParent PARENT_DIRECTORY)
if(NOT hasParent)
    cmake_minimum_required(VERSION 3.15)
    project(GetAway LANGUAGES CXX)
endif()

    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(srcFiles "GetAway/src/resourceStrings.cpp"
        "GetAway/src/clientChat.cpp"  "GetAway/src/deckSuit.cpp" "GetAway/src/serverChat.cpp"
        "GetAway/src/serverGetAway.cpp"
        "GetAway/src/sati.cpp" "GetAway/src/serverLobby.cpp" "GetAway/src/serverListener.cpp"
        "GetAway/src/clientLobbyPF.cpp" "GetAway/src/satiAndroid.cpp"
        "GetAway/src/clientGetAwayPF.cpp" "GetAway/src/home.cpp" "GetAway/src/homePF.cpp"
        "GetAway/src/serverLobbyPF.cpp" "GetAway/src/getAwayPData.cpp" "GetAway/src/clientChatPF.cpp"
        "GetAway/src/replacementStreamBuff.cpp" "GetAway/src/clientLobby.cpp"
        "GetAway/src/serverListenerPF.cpp" "GetAway/src/clientGetAway.cpp"
        "GetAway/src/constants.cpp" "GetAway/src/serverBluff.cpp" "GetAway/src/clientBluff.cpp"
        "GetAway/src/clientBluffPF.cpp" "GetAway/src/bluffPData.cpp")


add_library(GetAway-lib "${srcFiles}")

target_compile_definitions(GetAway-lib PUBLIC ASIO_STANDALONE)
target_include_directories(GetAway-lib PUBLIC GetAway/header)
target_include_directories(GetAway-lib PUBLIC asio/asio/include)
target_include_directories(GetAway-lib PUBLIC spdlog/include)

if (WIN32)
    target_compile_options(GetAway-lib PUBLIC -D_WIN32_WINNT=0x0A00)
elseif(UNIX AND NOT ANDROID)
    find_package(Threads REQUIRED)
    target_link_libraries(GetAway-lib PRIVATE Threads::Threads)
endif()


if(NOT ANDROID)
    add_executable(GetAway "GetAway/main.cpp")
    target_link_libraries(GetAway PRIVATE GetAway-lib)
endif()

if(UNIX AND NOT ANDROID)
    add_subdirectory(ClientTest)
endif()


