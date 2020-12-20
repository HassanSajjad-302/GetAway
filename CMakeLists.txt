cmake_minimum_required(VERSION 3.5)

project(GetAway LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable(GetAway "GetAway/main.cpp" "GetAway/src/serverListener.cpp"
        "GetAway/src/serverAuthManager.cpp" "GetAway/src/serverLobbyManager.cpp"
        "GetAway/src/serverGameManager.cpp" "GetAway/src/messageTypeEnums.cpp"
        "GetAway/src/ManagementFunctions.cpp" "GetAway/src/sati.cpp" "GetAway/src/replacementStreamBuff.cpp"
        "GetAway/src/playerData.cpp" "GetAway/src/constants.cpp" "GetAway/src/deckSuit.cpp"
        "GetAway/src/serverHome.cpp" "GetAway/src/serverPF.cpp")

target_compile_definitions(GetAway PRIVATE SERVERMACRO=1)
target_include_directories(GetAway PRIVATE "GetAway/header")


if (WIN32)
target_compile_options(GetAway PRIVATE -D_WIN32_WINNT=0x0A00)


set (BOOST_ROOT "D:/Files/Boost_Installed")
set (Boost_LIBRARY_DIR  "D:/Files/Boost_Installed/lib")
find_package(Boost 1.75.0 REQUIRED
        COMPONENTS system regex date_time
        PATHS "D:/Files/Boost_Installed/lib/cmake/Boost-1.75.0"
        NO_DEFAULT_PATH)

find_package(spdlog REQUIRED
        PATHS "D:/Files/spdlog_Installed_make/lib/cmake/spdlog"
        NO_DEFAULT_PATH)

else()

find_package(Threads REQUIRED)
target_link_libraries(GetAway PRIVATE Threads::Threads)


set(Boost_NO_SYSTEM_PATHS TRUE)
if (Boost_NO_SYSTEM_PATHS)
  set(BOOST_ROOT "/home/hassan/Music/Boost_Installed")
  set(BOOST_INCLUDE_DIRS "${BOOST_ROOT}/include")
  set(BOOST_LIBRARY_DIRS "${BOOST_ROOT}/lib")
endif (Boost_NO_SYSTEM_PATHS)
find_package(Boost 1.75.0 REQUIRED
)

find_package(spdlog REQUIRED
        PATHS "/home/hassan/Music/spdlog_Installed/lib/cmake/spdlog"
        NO_DEFAULT_PATH)

endif()

target_include_directories(GetAway PUBLIC ${Boost_INCLUDE_DIRS})
message("${Boost_SYSTEM_LIBRARY}")
message("${Boost_LIBRARIES}")
target_link_libraries(GetAway PRIVATE ${Boost_SYSTEM_LIBRARY})
target_link_libraries(GetAway PRIVATE spdlog::spdlog_header_only)


add_subdirectory(Client)

