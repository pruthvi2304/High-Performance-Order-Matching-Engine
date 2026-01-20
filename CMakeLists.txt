cmake_minimum_required(VERSION 3.22)
project(order_matching_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(
-O3
-march=native
-Wall
-Wextra
-Wpedantic
)

add_executable(ome
src/main.cpp
src/engine/OrderBook.cpp
)

target_include_directories(ome PRIVATE src)

# Enable testing
enable_testing()

# Find GTest
find_package(GTest REQUIRED)

# Add test executable
add_executable(ome_tests
tests/OrderBookMatchTest.cpp
src/engine/OrderBook.cpp
)

target_include_directories(ome_tests PRIVATE src)
target_link_libraries(ome_tests GTest::GTest GTest::Main)

# Register tests
add_test(NAME OrderBookTests COMMAND ome_tests)

