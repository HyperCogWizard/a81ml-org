cmake_minimum_required(VERSION 3.14)

# Add executable for MOSES demo
add_executable(moses-demo demo.c)

# Link with ggml
target_link_libraries(moses-demo ggml)

# Include directories
target_include_directories(moses-demo PRIVATE ../../include)

# Add OpenMP if available
find_package(OpenMP REQUIRED)
if(OpenMP_C_FOUND)
    target_link_libraries(moses-demo OpenMP::OpenMP_C)
endif()

# Add math library
target_link_libraries(moses-demo m)