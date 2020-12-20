set(CMAKE_CXX_STANDARD 20)

add_executable(Client "${CMAKE_CURRENT_SOURCE_DIR}/../Client/main.cpp" "../GetAway/src/clientAuthManager.cpp"
        "../GetAway/src/clientLobbyManager.cpp" "../GetAway/src/clientGameManager.cpp"
        "../GetAway/src/messageTypeEnums.cpp" "../GetAway/src/ManagementFunctions.cpp"
        "../GetAway/src/sati.cpp" "../GetAway/src/constants.cpp" "../GetAway/src/deckSuit.cpp"
        "../GetAway/src/lobbyPF.cpp" "../GetAway/src/gamePF.cpp" "../GetAway/src/clientHomePF.cpp"
        "../GetAway/src/messagePF.cpp" "../GetAway/src/clientHome.cpp")
target_compile_definitions(Client PRIVATE CLIENTMACRO=1)
target_include_directories(Client PRIVATE "../GetAway/header")

if(WIN32)

target_compile_options(Client PRIVATE -D_WIN32_WINNT=0x0A00)


set (BOOST_ROOT "D:/Files/Boost_Installed")
set (Boost_LIBRARY_DIR  "D:/Files/Boost_Installed/lib")
find_package(Boost 1.75.0 REQUIRED
        COMPONENTS system regex date_time
        PATHS "D:/Files/Boost_Installed/lib/cmake/Boost-1.75.0"
        NO_DEFAULT_PATH)

find_package(spdlog REQUIRED
        PATHS "D:/Files/spdlog_Installed/lib/cmake/spdlog"
        NO_DEFAULT_PATH)

else()

find_package(Threads REQUIRED)

set(Boost_NO_SYSTEM_PATHS TRUE)
if (Boost_NO_SYSTEM_PATHS)
  set(BOOST_ROOT "/home/hassan/Music/Boost_Installed")
  set(BOOST_INCLUDE_DIRS "${BOOST_ROOT}/include")
  set(BOOST_LIBRARY_DIRS "${BOOST_ROOT}/lib")
endif (Boost_NO_SYSTEM_PATHS)
find_package(Boost 1.75.0 REQUIRED)

find_package(spdlog REQUIRED
        PATHS "/home/hassan/Music/spdlog_Installed_make/lib/cmake/spdlog"
        NO_DEFAULT_PATH)
target_link_libraries(Client PRIVATE Threads::Threads)
endif()


target_link_libraries(Client PRIVATE spdlog::spdlog_header_only)
message(${Boost_SYSTEM_LIBRARY})
target_link_libraries(Client PRIVATE ${Boost_SYSTEM_LIBRARY})
target_include_directories(Client PRIVATE ${Boost_INCLUDE_DIRS})

