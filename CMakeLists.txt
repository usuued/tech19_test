cmake_minimum_required(VERSION 3.15)
project(counter_drone_parser VERSION 1.0 LANGUAGES CXX)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# Include directories
include_directories(include)

# Server executable
add_executable(drone_server
    src/main.cpp
    src/crc16.cpp
    src/serialization.cpp
    src/parser.cpp
    src/network_listener.cpp
    src/business_logic.cpp
)

target_link_libraries(drone_server pthread)

# Test client
add_subdirectory(client)

# Unit tests (optional)
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
