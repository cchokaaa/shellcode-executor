cmake_minimum_required(VERSION 3.20)
project(shellcode-executor
    VERSION 2.0.2
    DESCRIPTION "跨平台 Shellcode 执行器与漏洞利用框架 / Cross-Platform Shellcode Executor & Exploitation Framework"
    LANGUAGES C
)

# ──────────────────────────────────────────────
# 项目选项 / Project Options
# ──────────────────────────────────────────────
option(BUILD_SHARED_LIBS     "Build shared libraries"                   OFF)
option(ENABLE_TESTS          "Build unit tests"                         ON)
option(ENABLE_EXAMPLES       "Build example programs"                   ON)
option(ENABLE_ASAN           "Enable AddressSanitizer"                  OFF)
option(ENABLE_UBSAN          "Enable UndefinedBehaviorSanitizer"        OFF)
option(ENABLE_STATIC_ANALYSIS "Enable Clang-Tidy static analysis"       OFF)
option(ENABLE_COVERAGE       "Enable code coverage instrumentation"    OFF)
option(WITH_ENCRYPTION       "Enable encryption module (OpenSSL)"       OFF)
option(WITH_REMOTE           "Enable remote shellcode fetch module"     ON)

# ──────────────────────────────────────────────
# C Standard & Compiler Settings
# ──────────────────────────────────────────────
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /utf-8 /MP)
    set(CMAKE_C_FLAGS_RELEASE "/O2 /GL /GS-")
    set(CMAKE_C_FLAGS_DEBUG "/Od /RTC1")
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wstrict-prototypes)
    add_compile_options(-Wmissing-prototypes -Wconversion -Wformat=2)

    set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
    set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")

    if(ENABLE_ASAN)
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
    if(ENABLE_UBSAN)
        add_compile_options(-fsanitize=undefined)
        add_link_options(-fsanitize=undefined)
    endif()
    if(ENABLE_COVERAGE)
        add_compile_options(--coverage)
        add_link_options(--coverage)
    endif()
endif()

# ──────────────────────────────────────────────
# Platform Detection
# ──────────────────────────────────────────────
if(WIN32)
    add_definitions(-DPLATFORM_WINDOWS=1)
    set(PLATFORM_LIBS ws2_32)
elseif(APPLE)
    add_definitions(-DPLATFORM_MACOS=1)
    set(PLATFORM_LIBS)
else()
    add_definitions(-DPLATFORM_LINUX=1)
    set(PLATFORM_LIBS dl)
endif()

# ──────────────────────────────────────────────
# Include Directories
# ──────────────────────────────────────────────
include_directories(${CMAKE_SOURCE_DIR}/src)

# ──────────────────────────────────────────────
# Source Files
# ──────────────────────────────────────────────
set(LOADER_SRC
    src/loader/loader.c
)

set(ENCODER_SRC
    src/encoder/encoder.c
    src/encoder/xor.c
    src/encoder/base64.c
)

set(UTILS_SRC
    src/utils/utils.c
    src/utils/hex.c
    src/utils/memory_info.c
)

set(REMOTE_SRC
    src/remote/remote.c
)

set(CRYPTO_SRC
    src/crypto/crypto.c
)

# ──────────────────────────────────────────────
# Core Static Library
# ──────────────────────────────────────────────
add_library(scloader STATIC
    ${LOADER_SRC}
    ${ENCODER_SRC}
    ${UTILS_SRC}
    ${REMOTE_SRC}
    ${CRYPTO_SRC}
)

target_include_directories(scloader PUBLIC ${CMAKE_SOURCE_DIR}/src)

if(WITH_ENCRYPTION)
    find_package(OpenSSL REQUIRED)
    target_link_libraries(scloader PUBLIC OpenSSL::Crypto)
    target_compile_definitions(scloader PRIVATE WITH_ENCRYPTION=1)
endif()

if(WITH_REMOTE AND WIN32)
    target_link_libraries(scloader PUBLIC ${PLATFORM_LIBS})
endif()

# ──────────────────────────────────────────────
# Main Executable
# ──────────────────────────────────────────────
add_executable(scloader-cli src/main.c)
target_link_libraries(scloader-cli PRIVATE scloader)

# ──────────────────────────────────────────────
# Examples
# ──────────────────────────────────────────────
if(ENABLE_EXAMPLES)
    add_subdirectory(examples)
endif()

# ──────────────────────────────────────────────
# Tests
# ──────────────────────────────────────────────
if(ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ──────────────────────────────────────────────
# Install
# ──────────────────────────────────────────────
install(TARGETS scloader scloader-cli
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY src/ DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# ──────────────────────────────────────────────
# Custom Targets
# ──────────────────────────────────────────────
if(ENABLE_STATIC_ANALYSIS AND NOT MSVC)
    find_program(CLANG_TIDY NAMES clang-tidy)
    if(CLANG_TIDY)
        set(CMAKE_C_CLANG_TIDY ${CLANG_TIDY}
            -checks=clang-analyzer-*,bugprone-*,performance-*
        )
    endif()
endif()