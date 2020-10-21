file(GLOB src_files "${CMAKE_CURRENT_SOURCE_DIR}/../GetAway/src/*.cpp" )
message("src_files value is ${src_files}")
add_executable(Tests main.cpp ${src_files})

    
target_include_directories(Tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../GetAway/header")


find_package(Threads REQUIRED)
target_link_libraries(Tests Threads::Threads)


set(Boost_NO_SYSTEM_PATHS TRUE)
if (Boost_NO_SYSTEM_PATHS)
  set(BOOST_ROOT "/home/hassan/Music/Boost_Installed")
  set(BOOST_INCLUDE_DIRS "${BOOST_ROOT}/include")
  set(BOOST_LIBRARY_DIRS "${BOOST_ROOT}/lib")
endif (Boost_NO_SYSTEM_PATHS)
find_package(Boost 1.74.0 REQUIRED)

message(${Boost_INCLUDE_DIRS})
target_include_directories(Tests PUBLIC ${Boost_INCLUDE_DIRS})


find_package(Catch2 REQUIRED
        PATHS /home/hassan/Music/Catch2_Installed/lib/cmake/Catch2
        NO_DEFAULT_PATH)

    target_link_libraries(Tests Catch2::Catch2)
    target_include_directories(Tests PRIVATE /home/hassan/Music/Catch2_Installed/include/catch2/)
