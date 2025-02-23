# Root CMakeLists.txt
cmake_minimum_required(VERSION 3.14)
project(6502emu VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Setup output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find Google Test
find_package(GTest REQUIRED)

# Add compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall)
endif()

# Collect source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*main\\.cpp$")
file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Main executable
add_executable(6502emu src/main.cpp ${SOURCES})

# Test executable
enable_testing()
add_executable(run_tests ${SOURCES} ${TEST_SOURCES})
target_link_libraries(run_tests PRIVATE 
    GTest::GTest 
    GTest::Main
)

# Add tests to CTest
add_test(NAME UnitTests COMMAND run_tests)

# Install rules (optional)
install(TARGETS 6502emu
    RUNTIME DESTINATION bin
)
