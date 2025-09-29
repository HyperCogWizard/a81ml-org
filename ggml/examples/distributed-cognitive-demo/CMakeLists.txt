cmake_minimum_required(VERSION 3.14)

# Add basic test without RPC dependency
add_executable(basic-test
    basic-test.c
)

target_include_directories(basic-test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(basic-test
    ggml-base
    ${CMAKE_THREAD_LIBS_INIT}
    m  # Math library for sin/cos functions
)

# Add distributed cognitive architecture demo
add_executable(distributed-cognitive-demo
    demo.c
)

# Add real distributed communication demo
add_executable(real-distributed-demo
    real-distributed-demo.c
)

target_include_directories(distributed-cognitive-demo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

target_include_directories(real-distributed-demo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(distributed-cognitive-demo
    ggml-base
    ggml-rpc
    ${CMAKE_THREAD_LIBS_INIT}
    m  # Math library for sin/cos functions
)

target_link_libraries(real-distributed-demo
    ggml-base
    ggml-rpc
    ${CMAKE_THREAD_LIBS_INIT}
    m  # Math library for sin/cos functions
)

# Add recursive self-improvement demo
add_executable(recursive-self-improvement-demo
    recursive-self-improvement-demo.c
)

target_include_directories(recursive-self-improvement-demo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(recursive-self-improvement-demo
    ggml-base
    ${CMAKE_THREAD_LIBS_INIT}
    m  # Math library for sin/cos functions
)

# Add recursive self-improvement test
add_executable(test-recursive-self-improvement
    test-recursive-self-improvement.c
)

target_include_directories(test-recursive-self-improvement PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test-recursive-self-improvement
    ggml-base
    ${CMAKE_THREAD_LIBS_INIT}
    m  # Math library for sin/cos functions
)

# Add meta-meta-reasoning demo
add_executable(meta-meta-reasoning-demo
    meta-meta-reasoning-demo.c
)

target_include_directories(meta-meta-reasoning-demo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(meta-meta-reasoning-demo
    ggml-base
    ${CMAKE_THREAD_LIBS_INIT}
    m  # Math library for sin/cos functions
)

# Add meta-meta-reasoning test
add_executable(test-meta-meta-reasoning
    test-meta-meta-reasoning.c
)

target_include_directories(test-meta-meta-reasoning PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test-meta-meta-reasoning
    ggml-base
    ${CMAKE_THREAD_LIBS_INIT}
    m  # Math library for sin/cos functions
)

# Add comprehensive integration tests
add_executable(test-distributed-cognitive-integration
    test-integration.c
)

target_include_directories(test-distributed-cognitive-integration PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test-distributed-cognitive-integration
    ggml-base
    ggml-rpc
    ${CMAKE_THREAD_LIBS_INIT}
    m
)

# Set output directory
set_target_properties(basic-test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(distributed-cognitive-demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(real-distributed-demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(recursive-self-improvement-demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(test-recursive-self-improvement PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(meta-meta-reasoning-demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(test-meta-meta-reasoning PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(test-distributed-cognitive-integration PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Add compile definitions
target_compile_definitions(basic-test PRIVATE
    _GNU_SOURCE
)

target_compile_definitions(distributed-cognitive-demo PRIVATE
    _GNU_SOURCE
)

target_compile_definitions(real-distributed-demo PRIVATE
    _GNU_SOURCE
)

target_compile_definitions(recursive-self-improvement-demo PRIVATE
    _GNU_SOURCE
)

target_compile_definitions(test-recursive-self-improvement PRIVATE
    _GNU_SOURCE
)

target_compile_definitions(meta-meta-reasoning-demo PRIVATE
    _GNU_SOURCE
)

target_compile_definitions(test-meta-meta-reasoning PRIVATE
    _GNU_SOURCE
)

target_compile_definitions(test-distributed-cognitive-integration PRIVATE
    _GNU_SOURCE
)