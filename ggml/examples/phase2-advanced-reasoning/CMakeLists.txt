cmake_minimum_required(VERSION 3.14)

set(TARGET phase2-advanced-reasoning)

add_executable(${TARGET} main.c)
add_executable(phase2-simple-demo simple-demo.c)

target_link_libraries(${TARGET} PRIVATE ggml-base)
target_include_directories(${TARGET} PRIVATE ../../include)

target_link_libraries(phase2-simple-demo PRIVATE ggml-base)
target_include_directories(phase2-simple-demo PRIVATE ../../include)

if (WIN32)
    target_link_libraries(${TARGET} PRIVATE ws2_32)
    target_link_libraries(phase2-simple-demo PRIVATE ws2_32)
endif()

target_compile_definitions(${TARGET} PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
)

target_compile_definitions(phase2-simple-demo PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
)

# Set C standard for better compatibility
set_property(TARGET ${TARGET} PROPERTY C_STANDARD 99)
set_property(TARGET ${TARGET} PROPERTY C_STANDARD_REQUIRED ON)

set_property(TARGET phase2-simple-demo PROPERTY C_STANDARD 99)
set_property(TARGET phase2-simple-demo PROPERTY C_STANDARD_REQUIRED ON)