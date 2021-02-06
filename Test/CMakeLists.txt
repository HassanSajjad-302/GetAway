add_executable(Test GetAway/main.cpp GetAway/GetAwayTest.cpp)
target_include_directories(Test PRIVATE GetAway)
target_include_directories(Test PUBLIC ../asio/asio/include)

target_link_libraries(Test PRIVATE Threads::Threads)