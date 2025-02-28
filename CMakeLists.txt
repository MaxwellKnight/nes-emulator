cmake_minimum_required(VERSION 3.10)
project(nes_emulator)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_WASM "Build WebAssembly version" OFF)
option(BUILD_TESTS "Build tests" ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Only include GTest for non-WASM builds with tests enabled
if(NOT BUILD_WASM AND BUILD_TESTS)
    find_package(GTest REQUIRED)
endif()

# Define source files
set(CORE_SOURCES
    src/cpu.cpp
    src/bus.cpp
    src/debugger.cpp
)

if(BUILD_WASM)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s WASM=1 -s ASSERTIONS=1 -s ALLOW_MEMORY_GROWTH=1")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']")
  
  # List all your exported functions here
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s EXPORTED_FUNCTIONS=['_debugger_step','_debugger_run','_debugger_stop','_debugger_reset','_debugger_is_running','_debugger_add_breakpoint','_debugger_remove_breakpoint','_debugger_clear_breakpoints','_debugger_get_register_a','_debugger_get_register_x','_debugger_get_register_y','_debugger_get_register_sp','_debugger_get_register_pc','_debugger_get_register_status','_debugger_get_status_flag','_debugger_read_memory','_debugger_write_memory','_debugger_get_instruction_count','_debugger_get_cycle_count','_debugger_disassemble_around_pc','_debugger_disassemble_range','_main_loop','_malloc','_free']")
  
  # This option can help with signature mismatches
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s DISABLE_EXCEPTION_CATCHING=0")
  
  # For debugging purposes
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g4 -s DEMANGLE_SUPPORT=1")
endif()

if(BUILD_WASM)
    # WebAssembly build
    set(SOURCES
        ${CORE_SOURCES}
        src/wasm_main.cpp
    )

    # Add executable
    add_executable(nes_debugger ${SOURCES})

    # Emscripten specific settings
    set(CMAKE_EXECUTABLE_SUFFIX ".js")

    # Export the C functions to JavaScript
    set_target_properties(nes_debugger PROPERTIES
        LINK_FLAGS "-s WASM=1 -s EXPORTED_RUNTIME_METHODS=['cwrap','ccall'] -s ALLOW_MEMORY_GROWTH=1 -s EXPORT_ALL=1 -s EXPORTED_FUNCTIONS=['_debugger_step','_debugger_run','_debugger_stop','_debugger_reset','_debugger_is_running','_debugger_add_breakpoint','_debugger_remove_breakpoint','_debugger_clear_breakpoints','_debugger_get_register_a','_debugger_get_register_x','_debugger_get_register_y','_debugger_get_register_sp','_debugger_get_register_pc','_debugger_get_register_status','_debugger_get_status_flag','_debugger_read_memory','_debugger_write_memory','_debugger_get_instruction_count','_debugger_get_cycle_count','_debugger_disassemble_around_pc','_debugger_disassemble_range','_malloc','_free'] -s NO_EXIT_RUNTIME=1 -s ENVIRONMENT='web' -O3"
    )
else()
    # Regular build - check if main.cpp exists before building the executable
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
        set(SOURCES
            ${CORE_SOURCES}
            src/main.cpp
        )

        # Add executable
        add_executable(nes_emulator ${SOURCES})
    endif()

    # Tests are only built if requested
    if(BUILD_TESTS)
        enable_testing()

        # Add test executables here - one for each test file
        file(GLOB TEST_SOURCES tests/cpu_test_*.cpp)
        foreach(TEST_FILE ${TEST_SOURCES})
            # Extract the file name without extension
            get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
            # Create an executable for each test file
            add_executable(${TEST_NAME} ${TEST_FILE} ${CORE_SOURCES})
            # Link with GTest
            target_link_libraries(${TEST_NAME} GTest::GTest GTest::Main)
            # Add to CTest
            add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
        endforeach()
    endif()
endif()
