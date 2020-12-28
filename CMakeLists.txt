get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
else()
    cmake_minimum_required(VERSION 3.5)

    project(GetAway LANGUAGES CXX)
endif()

    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(srcFiles "GetAway/main.cpp" "GetAway/src/serverListener.cpp"
        "GetAway/src/serverAuthManager.cpp" "GetAway/src/serverLobbyManager.cpp"
        "GetAway/src/messageTypeEnums.cpp"
        "GetAway/src/sati.cpp" "GetAway/src/replacementStreamBuff.cpp"
        "GetAway/src/playerData.cpp" "GetAway/src/constants.cpp" "GetAway/src/deckSuit.cpp"
        "GetAway/src/serverHome.cpp" "GetAway/src/serverPF.cpp" "GetAway/src/resourceStrings.cpp"
        "GetAway/src/androidServerMain.cpp" "GetAway/src/satiAndroid.cpp" "GetAway/header")

if(NOT ANDROID)
    add_executable(GetAway "${srcFiles}")
else()
    add_library(GetAway "${srcFiles}")
endif()

target_compile_definitions(GetAway PUBLIC SERVERMACRO=1 ASIO_STANDALONE)

target_include_directories(GetAway PUBLIC GetAway/header)
target_include_directories(GetAway PUBLIC asio/asio/include)


if (WIN32)
    target_compile_options(GetAway PRIVATE -D_WIN32_WINNT=0x0A00)

    find_package(spdlog REQUIRED
            PATHS "D:/Files/spdlog_Installed_make/lib/cmake/spdlog"
            NO_DEFAULT_PATH)

    target_link_libraries(GetAway PRIVATE spdlog::spdlog_header_only)

elseif(ANDROID)


else()

    find_package(Threads REQUIRED)
    target_link_libraries(GetAway PRIVATE Threads::Threads)

    find_package(spdlog REQUIRED
            PATHS "/home/hassan/Music/spdlog_Installed/lib/cmake/spdlog"
            NO_DEFAULT_PATH)
    target_link_libraries(GetAway PRIVATE spdlog::spdlog_header_only)
endif()

get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
    message(STATUS "Has a parent scope.")
else()
    add_subdirectory(Client)
endif()

