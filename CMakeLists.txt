cmake_minimum_required(VERSION 3.5)

project(GetAway LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(GetAway "GetAway/main.cpp" "GetAway/src/serverListener.cpp"
        "GetAway/src/serverAuthManager.cpp" "GetAway/src/serverLobbyManager.cpp"
        "GetAway/src/serverGameManager.cpp" "GetAway/src/messageTypeEnums.cpp"
        "GetAway/src/ManagementFunctions.cpp" "GetAway/src/sati.cpp" "GetAway/src/replacementStreamBuff.cpp"
        "GetAway/src/playerData.cpp" "GetAway/src/constants.cpp" "GetAway/src/deckSuit.cpp"
        "GetAway/src/serverHome.cpp" "GetAway/src/serverPF.cpp" "GetAway/src/resourceStrings.cpp")

target_compile_definitions(GetAway PRIVATE SERVERMACRO=1)
target_include_directories(GetAway PRIVATE "GetAway/header")


if (WIN32)
target_compile_options(GetAway PRIVATE -DASIO_STANDALONE -D_WIN32_WINNT=0x0A00)

target_include_directories(GetAway PRIVATE asio/asio/include)

find_package(spdlog REQUIRED
        PATHS "D:/Files/spdlog_Installed_make/lib/cmake/spdlog"
        NO_DEFAULT_PATH)
target_link_libraries(GetAway PRIVATE spdlog::spdlog_header_only)

else()

find_package(Threads REQUIRED)
target_link_libraries(GetAway PRIVATE Threads::Threads)

find_package(spdlog REQUIRED
        PATHS "/home/hassan/Music/spdlog_Installed/lib/cmake/spdlog"
        NO_DEFAULT_PATH)

endif()

add_subdirectory(Client)

