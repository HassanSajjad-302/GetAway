set(CMAKE_CXX_STANDARD 20)

add_executable(Client "${CMAKE_CURRENT_SOURCE_DIR}/../Client/main.cpp" "../GetAway/src/clientAuthManager.cpp"
        "../GetAway/src/clientLobbyManager.cpp" "../GetAway/src/clientGameManager.cpp"
        "../GetAway/src/messageTypeEnums.cpp" "../GetAway/src/ManagementFunctions.cpp"
        "../GetAway/src/sati.cpp" "../GetAway/src/constants.cpp" "../GetAway/src/deckSuit.cpp"
        "../GetAway/src/lobbyPF.cpp" "../GetAway/src/gamePF.cpp" "../GetAway/src/clientHomePF.cpp"
        "../GetAway/src/messagePF.cpp" "../GetAway/src/clientHome.cpp"
        "../GetAway/src/resourceStrings.cpp")
target_compile_definitions(Client PRIVATE CLIENTMACRO=1)
target_include_directories(Client PRIVATE "../GetAway/header")

if (WIN32)
  target_compile_options(Client PRIVATE -DASIO_STANDALONE -D_WIN32_WINNT=0x0A00)

  target_include_directories(Client PRIVATE D:/Files/Check/asio-asio-1-18-0/asio/include)

  find_package(spdlog REQUIRED
         PATHS "D:/Files/spdlog_Installed_make/lib/cmake/spdlog"
        NO_DEFAULT_PATH)
  target_link_libraries(Client PRIVATE spdlog::spdlog_header_only)
else()

  find_package(Threads REQUIRED)
  target_link_libraries(Client PRIVATE Threads::Threads)

  find_package(spdlog REQUIRED
          PATHS "/home/hassan/Music/spdlog_Installed/lib/cmake/spdlog"
          NO_DEFAULT_PATH)

endif()

