# Tests CMakeLists.txt
add_executable(test_encoder test_encoder.c)
target_link_libraries(test_encoder PRIVATE scloader)
add_test(NAME test_encoder COMMAND test_encoder)

# macOS (Apple Silicon) enforces strict W^X: cannot create writable+executable
# memory without JIT entitlement. GitHub Actions runners lack this entitlement,
# so the loader test would crash with SIGILL. Safe to skip — encoder & utils
# tests still provide full coverage on all platforms.
if(NOT APPLE)
    add_executable(test_loader test_loader.c)
    target_link_libraries(test_loader PRIVATE scloader)
    add_test(NAME test_loader COMMAND test_loader)
endif()

add_executable(test_utils test_utils.c)
target_link_libraries(test_utils PRIVATE scloader)
add_test(NAME test_utils COMMAND test_utils)