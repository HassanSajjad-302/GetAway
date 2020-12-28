  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(srcFiles "../Client/main.cpp" "../GetAway/src/clientAuthManager.cpp"
        "../GetAway/src/clientLobbyManager.cpp"
        "../GetAway/src/messageTypeEnums.cpp"
        "../GetAway/src/sati.cpp" "../GetAway/src/constants.cpp" "../GetAway/src/deckSuit.cpp"
        "../GetAway/src/lobbyPF.cpp" "../GetAway/src/gamePF.cpp" "../GetAway/src/clientHomePF.cpp"
        "../GetAway/src/messagePF.cpp" "../GetAway/src/clientHome.cpp"
        "../GetAway/src/resourceStrings.cpp" "../GetAway/src/androidClientMain.cpp"
        "../GetAway/src/satiAndroid.cpp"
        )

if(NOT ANDROID)
  add_executable(Client "${srcFiles}")
else()
  add_library(Client "${srcFiles}")
endif()

target_compile_definitions(Client PUBLIC CLIENTMACRO=1 ASIO_STANDALONE)


  target_include_directories(Client PUBLIC ../GetAway/header)
  target_include_directories(Client PUBLIC ../asio/asio/include)


if (WIN32)
  target_compile_options(Client PRIVATE -D_WIN32_WINNT=0x0A00)

  find_package(spdlog REQUIRED
         PATHS "D:/Files/spdlog_Installed_make/lib/cmake/spdlog"
        NO_DEFAULT_PATH)

  target_link_libraries(Client PRIVATE spdlog::spdlog_header_only)

elseif(ANDROID)


else()

  find_package(Threads REQUIRED)
  target_link_libraries(Client PRIVATE Threads::Threads)

  find_package(spdlog REQUIRED
          PATHS "/home/hassan/Music/spdlog_Installed/lib/cmake/spdlog"
          NO_DEFAULT_PATH)
  target_link_libraries(Client PRIVATE spdlog::spdlog_header_only)

endif()

