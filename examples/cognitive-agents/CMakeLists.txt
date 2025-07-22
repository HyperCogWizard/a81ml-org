cmake_minimum_required(VERSION 3.14)

# Add cognitive agents example
add_executable(cognitive-agents-demo
    demo.c
    cognitive-agent.c
)

target_include_directories(cognitive-agents-demo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(cognitive-agents-demo
    ggml-base
    ggml
    ${CMAKE_THREAD_LIBS_INIT}
    m  # Math library for sin/cos functions
)

# Set output directory
set_target_properties(cognitive-agents-demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Add compile definitions
target_compile_definitions(cognitive-agents-demo PRIVATE
    _GNU_SOURCE
)