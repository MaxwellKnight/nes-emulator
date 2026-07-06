cmake_minimum_required(VERSION 3.10)
project(NES_Emulator VERSION 0.1.5)

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
    src/ppu.cpp
    src/apu.cpp
    src/cartridge.cpp
    src/mapper.cpp
    src/mapper_zero.cpp
    src/mapper_mmc1.cpp
    src/mapper_uxrom.cpp
    src/mapper_cnrom.cpp
    src/mapper_mmc3.cpp
    src/debugger.cpp
    src/palette.cpp
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
        "-s WASM=1 -s MODULARIZE=1 -s EXPORT_ES6=1 -s EXPORT_NAME='CPUEmulator' -s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap','UTF8ToString','writeAsciiToMemory','HEAPU8','HEAPF32'] -s NO_EXIT_RUNTIME=1 -s EXPORTED_FUNCTIONS=['_debugger_step','_debugger_run','_debugger_stop','_debugger_reset','_debugger_is_running','_debugger_add_breakpoint','_debugger_remove_breakpoint','_debugger_clear_breakpoints','_debugger_get_register_a','_debugger_get_register_x','_debugger_get_register_y','_debugger_get_register_sp','_debugger_get_register_pc','_debugger_get_register_status','_debugger_get_status_flag','_debugger_read_memory','_debugger_write_memory','_debugger_get_instruction_count','_debugger_get_cycle_count','_debugger_set_pc','_debugger_disassemble_around_pc','_debugger_disassemble_range','_load_rom','_get_framebuffer_ptr','_get_framebuffer_len','_get_frame_count','_run_frame','_ppu_render_pattern_table','_get_nametable_ptr','_get_palette_ram_ptr','_get_oam_ptr','_set_controller','_audio_available','_audio_drain','_ppu_get_ctrl','_ppu_get_mask','_ppu_get_status','_ppu_get_scanline','_malloc','_free']")

    # Export main as CPU_wasm
    set_target_properties(cpu_wasm PROPERTIES
        OUTPUT_NAME "cpu_wasm"
        SUFFIX ".js"
        LINK_FLAGS "${EM_LINK_FLAGS}"
    )

    # Copy the generated files to the web ESM bridge directory
    add_custom_command(TARGET cpu_wasm POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            ${CMAKE_SOURCE_DIR}/web/src/wasm/generated
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_BINARY_DIR}/cpu_wasm.js
            ${CMAKE_BINARY_DIR}/cpu_wasm.wasm
            ${CMAKE_SOURCE_DIR}/web/src/wasm/generated/
        COMMENT "Copying WebAssembly files to web/src/wasm/generated"
    )
else()
    # Native build configuration for testing
    message(STATUS "Configuring for native build")

    # Headless environment: a C-ABI shared library (loadable from Python via
    # ctypes) plus a recorder tool that writes .nesmovie files.
    option(BUILD_ENV "Build the headless env library and recorder" ON)
    if(BUILD_ENV)
        set_property(TARGET cpu_core PROPERTY POSITION_INDEPENDENT_CODE ON)
        add_library(nesenv SHARED src/nes_env.cpp)
        target_link_libraries(nesenv cpu_core)
        add_executable(record_demo tools/record_demo.cpp src/nes_env.cpp)
        target_link_libraries(record_demo cpu_core)
    endif()

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

        # If no system/CI GTest is available, fetch and build it from source.
        # This makes `ctest --test-dir native_build` work on a clean machine
        # (e.g. local macOS without `brew install googletest`).
        if(NOT GTEST_FOUND AND NOT DEFINED ENV{GTEST_ROOT} AND NOT EXISTS "/usr/lib/libgtest.a")
            message(STATUS "GTest not found; fetching googletest via FetchContent")
            include(FetchContent)
            FetchContent_Declare(
                googletest
                GIT_REPOSITORY https://github.com/google/googletest.git
                GIT_TAG v1.14.0
            )
            # Prevent googletest from overriding our compiler/linker options on Windows
            set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
            FetchContent_MakeAvailable(googletest)
            set(NES_USE_FETCHED_GTEST ON)
        endif()

        # Function to add test executables
        function(add_cpu_test test_name test_file)
            add_executable(${test_name} ${test_file})
            target_link_libraries(${test_name} cpu_core)

            if(NES_USE_FETCHED_GTEST)
                target_link_libraries(${test_name} GTest::gtest GTest::gtest_main)
                if(UNIX)
                    target_link_libraries(${test_name} pthread)
                endif()
            elseif(GTEST_FOUND)
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
        add_cpu_test(bus_test_ppu tests/bus_test_ppu.cpp)
        add_cpu_test(debugger_test_run_frame tests/debugger_test_run_frame.cpp)
        add_cpu_test(ppu_test_init tests/ppu_test_init.cpp)
        add_cpu_test(ppu_test_registers tests/ppu_test_registers.cpp)
        add_cpu_test(ppu_test_memory tests/ppu_test_memory.cpp)
        add_cpu_test(ppu_test_loopy tests/ppu_test_loopy.cpp)
        add_cpu_test(ppu_test_timing tests/ppu_test_timing.cpp)
        add_cpu_test(ppu_test_render tests/ppu_test_render.cpp)
        add_cpu_test(ppu_test_sprites tests/ppu_test_sprites.cpp)

        # PPU foundation tests
        add_cpu_test(palette_test tests/palette_test.cpp)
        add_cpu_test(cpu_test_nmi tests/cpu_test_nmi.cpp)
        add_cpu_test(cartridge_test_ines tests/cartridge_test_ines.cpp)
        add_cpu_test(debugger_test_rom_boot tests/debugger_test_rom_boot.cpp)
        add_cpu_test(controller_test tests/controller_test.cpp)
        add_cpu_test(mapper_test tests/mapper_test.cpp)
        add_cpu_test(apu_test tests/apu_test.cpp)
        add_cpu_test(cpu_test_illegal tests/cpu_test_illegal.cpp)

        # Headless C-ABI env test (compiles the ABI translation unit in directly).
        add_cpu_test(nes_env_test tests/nes_env_test.cpp)
        target_sources(nes_env_test PRIVATE src/nes_env.cpp)

        # Conformance suite against canonical NES test ROMs (fetched on demand
        # by tests/roms/fetch_test_roms.sh; cases SKIP when a ROM is absent).
        add_cpu_test(conformance_test tests/conformance_test.cpp)
        target_compile_definitions(conformance_test PRIVATE
            NES_TEST_ROM_DIR="${CMAKE_SOURCE_DIR}/tests/roms")
    endif()
endif()
