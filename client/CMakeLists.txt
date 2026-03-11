cmake_minimum_required(VERSION 3.15)

# Test client executable
add_executable(test_client
    test_client.cpp
    ../src/crc16.cpp
    ../src/serialization.cpp
)

target_include_directories(test_client PRIVATE ../include)

# Link libraries
if(WIN32)
    target_link_libraries(test_client ws2_32)
else()
    target_link_libraries(test_client pthread)
endif()
