add_executable(Tests main.cpp)


find_package(Catch2 REQUIRED
        PATHS /home/hassan/Music/Catch2_Installed/lib/cmake/Catch2
        NO_DEFAULT_PATH)
        
    target_link_libraries(Tests Catch2::Catch2)
    target_include_directories(Tests PRIVATE /home/hassan/Music/Catch2_Installed/include/catch2/)
    
