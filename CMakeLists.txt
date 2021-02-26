get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
else()
    cmake_minimum_required(VERSION 3.5)

    project(GetAway LANGUAGES CXX)
endif()

    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(srcFiles "GetAway/main.cpp" "GetAway/src/androidClientMain.cpp" "GetAway/src/resourceStrings.cpp"
        "GetAway/src/clientChat.cpp" "GetAway/src/androidServerMain.cpp" "GetAway/src/deckSuit.cpp"
        "GetAway/src/serverChat.cpp" "GetAway/src/serverGetAway.cpp"
        "GetAway/src/sati.cpp" "GetAway/src/serverLobby.cpp" "GetAway/src/serverListener.cpp"
        "GetAway/src/clientLobbyPF.cpp" "GetAway/src/satiAndroid.cpp"
        "GetAway/src/clientGetAwayPF.cpp" "GetAway/src/home.cpp" "GetAway/src/homePF.cpp"
        "GetAway/src/serverLobbyPF.cpp" "GetAway/src/getAwayPData.cpp" "GetAway/src/clientChatPF.cpp"
        "GetAway/src/replacementStreamBuff.cpp" "GetAway/src/clientLobby.cpp"
        "GetAway/src/serverListenerPF.cpp" "GetAway/src/clientGetAway.cpp"
        "GetAway/src/constants.cpp" "GetAway/src/serverBluff.cpp" "GetAway/src/clientBluff.cpp"
        "GetAway/src/clientBluffPF.cpp" "GetAway/src/bluffPData.cpp")

if(NOT ANDROID)
    add_executable(GetAway "${srcFiles}")
else ()
    add_library(GetAway "${srcFiles}")
endif ()

target_compile_definitions(GetAway PUBLIC SERVERMACRO=1 ASIO_STANDALONE)

target_include_directories(GetAway PUBLIC GetAway/header)
target_include_directories(GetAway PUBLIC asio/asio/include)
#target_include_directories(GetAway PUBLIC spdlog/include)

#if this gives error, comment it and uncomment above line. It will result in a little slow compilation however.
find_package(spdlog REQUIRED)
target_link_libraries(GetAway PUBLIC spdlog::spdlog)

if (WIN32 AND NOT ANDROID)
    target_compile_options(GetAway PRIVATE -D_WIN32_WINNT=0x0A00)

    find_package(spdlog REQUIRED
            PATHS "D:/Files/spdlog_Installed_make/lib/cmake/spdlog"
            NO_DEFAULT_PATH)

    target_link_libraries(GetAway PRIVATE spdlog::spdlog_header_only)
else ()
    find_package(Threads REQUIRED)
    target_link_libraries(GetAway PRIVATE Threads::Threads)
endif()

get_directory_property(hasParent PARENT_DIRECTORY)
add_subdirectory(ClientTest)
