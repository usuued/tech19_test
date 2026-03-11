cmake_minimum_required(VERSION 3.15)

# Simple unit tests (without external framework)
add_executable(run_tests
    test_main.cpp
    ../src/crc16.cpp
    ../src/serialization.cpp
)

target_include_directories(run_tests PRIVATE ../include)
target_link_libraries(run_tests pthread)

add_test(NAME UnitTests COMMAND run_tests)
