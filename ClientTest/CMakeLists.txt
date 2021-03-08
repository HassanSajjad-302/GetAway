set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)


add_executable(ClientTest "main.cpp")
target_link_libraries(ClientTest PRIVATE GetAway-lib)