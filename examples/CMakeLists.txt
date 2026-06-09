# Examples CMakeLists.txt
add_executable(basic_loader basic_loader.c)
target_link_libraries(basic_loader PRIVATE scloader)

add_executable(xor_encoded xor_encoded.c)
target_link_libraries(xor_encoded PRIVATE scloader)

add_executable(remote_fetch remote_fetch.c)
target_link_libraries(remote_fetch PRIVATE scloader)