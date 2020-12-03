
file(GLOB src_files "${CMAKE_CURRENT_SOURCE_DIR}/../GetAway/src/*.cpp" )
message("src_files value is ${src_files}")
#add_executable(Client "${CMAKE_CURRENT_SOURCE_DIR}/../Client/main.cpp" ${src_files})
add_executable(Client "${CMAKE_CURRENT_SOURCE_DIR}/../Client/main.cpp" "../GetAway/src/clientAuthManager.cpp"
        "../GetAway/src/clientLobbyManager.cpp" "../GetAway/src/clientGameManager.cpp"
        "../GetAway/src/messageTypeEnums.cpp" "../GetAway/src/ManagementFunctions.cpp"
        "../GetAway/src/sati.cpp" "../GetAway/src/constants.cpp" "../GetAway/src/deckSuit.cpp")
target_include_directories(Client PRIVATE "../GetAway/header")
if (USE_GCC)
target_compile_options(Client PRIVATE -fcoroutines)
endif()
if(USE_GCC)
    target_compile_options(Client "-Wunused")
endif()


find_package(Threads REQUIRED)
target_link_libraries(Client PRIVATE Threads::Threads)

set(Boost_NO_SYSTEM_PATHS TRUE)
if (Boost_NO_SYSTEM_PATHS)
  set(BOOST_ROOT "/home/hassan/Music/Boost_Installed")
  set(BOOST_INCLUDE_DIRS "${BOOST_ROOT}/include")
  set(BOOST_LIBRARY_DIRS "${BOOST_ROOT}/lib")
endif (Boost_NO_SYSTEM_PATHS)
find_package(Boost 1.74.0 REQUIRED)

message(${Boost_INCLUDE_DIRS})
target_include_directories(Client PRIVATE ${Boost_INCLUDE_DIRS})


find_package(spdlog REQUIRED
        PATHS "/home/hassan/Music/spdlog_Installed/lib/cmake/spdlog"
        NO_DEFAULT_PATH)

target_link_libraries(Client PRIVATE spdlog::spdlog)


