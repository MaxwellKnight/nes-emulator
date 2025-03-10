cmake_minimum_required(VERSION 3.10)
project(NES_Emulator VERSION 0.1.4)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compile definitions for required headers
add_compile_definitions(
    NEED_CSTDDEF
    NEED_STDEXCEPT
)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files for the main library
set(SOURCES
    src/bus.cpp
    src/cpu.cpp
    src/debugger.cpp
)

# Add library with the core functionality
add_library(cpu_core STATIC ${SOURCES})

# Detect if we're compiling for WebAssembly with Emscripten
if(EMSCRIPTEN)
    # WebAssembly build configuration
    message(STATUS "Configuring for WebAssembly build")

    # Add WebAssembly executable
    add_executable(cpu_wasm src/wasm_main.cpp)
    target_link_libraries(cpu_wasm cpu_core)

    # Emscripten-specific flags
    set(EM_LINK_FLAGS 
        "-s WASM=1 -s MODULARIZE=1 -s EXPORT_NAME='CPUEmulator' -s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap','UTF8ToString','writeAsciiToMemory'] -s NO_EXIT_RUNTIME=1 -s EXPORTED_FUNCTIONS=['_debugger_step','_debugger_run','_debugger_stop','_debugger_reset','_debugger_is_running','_debugger_add_breakpoint','_debugger_remove_breakpoint','_debugger_clear_breakpoints','_debugger_get_register_a','_debugger_get_register_x','_debugger_get_register_y','_debugger_get_register_sp','_debugger_get_register_pc','_debugger_get_register_status','_debugger_get_status_flag','_debugger_read_memory','_debugger_write_memory','_debugger_get_instruction_count','_debugger_get_cycle_count','_debugger_set_pc','_debugger_disassemble_around_pc','_debugger_disassemble_range','_debugger_print_state']")

    # Export main as CPU_wasm
    set_target_properties(cpu_wasm PROPERTIES
        OUTPUT_NAME "cpu_wasm"
        SUFFIX ".js"
        LINK_FLAGS "${EM_LINK_FLAGS}"
    )

    # Copy the generated files to the web directory
    add_custom_command(TARGET cpu_wasm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_BINARY_DIR}/cpu_wasm.js
            ${CMAKE_BINARY_DIR}/cpu_wasm.wasm
            ${CMAKE_SOURCE_DIR}/web/js/
        COMMENT "Copying WebAssembly files to web directory"
    )
else()
    # Native build configuration for testing
    message(STATUS "Configuring for native build")

    # Add test support (optional)
    option(BUILD_TESTS "Build test executables" ON)

    if(BUILD_TESTS)
        # Enable testing
        enable_testing()

        # Make GTest truly optional
        option(USE_SYSTEM_GTEST "Use system installed GTest" ON)

        if(USE_SYSTEM_GTEST)
            # Try to find GTest package, but make it optional
            find_package(GTest QUIET)
        endif()

        # Function to add test executables
        function(add_cpu_test test_name test_file)
            add_executable(${test_name} ${test_file})
            target_link_libraries(${test_name} cpu_core)

            if(GTEST_FOUND)
                target_include_directories(${test_name} PRIVATE ${GTEST_INCLUDE_DIRS})
                target_link_libraries(${test_name} ${GTEST_BOTH_LIBRARIES})
                if(UNIX)
                    target_link_libraries(${test_name} pthread)
                endif()
            else()
                # If GTest is not found, we'll try to use manually specified paths
                # This is necessary for CI environments
                if(DEFINED ENV{GTEST_ROOT})
                    message(STATUS "Using GTEST_ROOT from environment: $ENV{GTEST_ROOT}")
                    target_include_directories(${test_name} PRIVATE $ENV{GTEST_ROOT}/include)
                    if(UNIX)
                        target_link_libraries(${test_name} 
                            $ENV{GTEST_ROOT}/lib/libgtest.a 
                            $ENV{GTEST_ROOT}/lib/libgtest_main.a 
                            pthread)
                    else()
                        # Windows paths
                        target_link_libraries(${test_name} 
                            $ENV{GTEST_ROOT}/lib/gtest.lib 
                            $ENV{GTEST_ROOT}/lib/gtest_main.lib)
                    endif()
                elseif(EXISTS "/usr/lib/libgtest.a")
                    # Fallback for Ubuntu systems
                    message(STATUS "Using system GTest libraries from /usr/lib")
                    target_include_directories(${test_name} PRIVATE /usr/include)
                    target_link_libraries(${test_name} /usr/lib/libgtest.a /usr/lib/libgtest_main.a pthread)
                else()
                    message(WARNING "GTest not found and no fallback available. Tests will compile but may not link properly.")
                endif()
            endif()

            add_test(NAME ${test_name} COMMAND ${test_name})
        endfunction()

        # Add all test executables
        add_cpu_test(cpu_test_addr_mode tests/cpu_test_addr_mode.cpp)
        add_cpu_test(cpu_test_arithmetic tests/cpu_test_arithmetic.cpp)
        add_cpu_test(cpu_test_branch tests/cpu_test_branch.cpp)
        add_cpu_test(cpu_test_control_flow tests/cpu_test_control_flow.cpp)
        add_cpu_test(cpu_test_flags tests/cpu_test_flags.cpp)
        add_cpu_test(cpu_test_increment_decrement tests/cpu_test_increment_decrement.cpp)
        add_cpu_test(cpu_test_init tests/cpu_test_init.cpp)
        add_cpu_test(cpu_test_load tests/cpu_test_load.cpp)
        add_cpu_test(cpu_test_logic tests/cpu_test_logic.cpp)
        add_cpu_test(cpu_test_nop tests/cpu_test_nop.cpp)
        add_cpu_test(cpu_test_shift_rotate tests/cpu_test_shift_rotate.cpp)
        add_cpu_test(cpu_test_stack tests/cpu_test_stack.cpp)
        add_cpu_test(cpu_test_store tests/cpu_test_store.cpp)
        add_cpu_test(cpu_test_transfer tests/cpu_test_transfer.cpp)
    endif()
endif()
