cmake_minimum_required(VERSION 3.10)
project(CPU_Emulator VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
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


# Detect if we're compiling for WebAssembly with Emscripten
if(EMSCRIPTEN)
    # WebAssembly build configuration
    message(STATUS "Configuring for WebAssembly build")
    
    # Add library with the core functionality
    add_library(cpu_core STATIC ${SOURCES})
    
    # Add WebAssembly executable
    add_executable(cpu_wasm src/wasm_main.cpp)
    target_link_libraries(cpu_wasm cpu_core)
    
    # Emscripten-specific flags - use a simpler approach
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
    
    # Add library with the core functionality
    add_library(cpu_core STATIC ${SOURCES})
    
    # Add Google Test
    enable_testing()
    
    # Find GTest package
    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})

    # Function to add test executables
    function(add_cpu_test test_name test_file)
        add_executable(${test_name} ${test_file})
        target_link_libraries(${test_name} cpu_core ${GTEST_BOTH_LIBRARIES} pthread)
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
