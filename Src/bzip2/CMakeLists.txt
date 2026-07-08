cmake_minimum_required(VERSION 3.12)

project(bzip2 C)

set(CMAKE_C_STANDARD 99)

# 출력 디렉터리
set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/../../Build/LinuxRelease")
set(INC_DIR    "${CMAKE_SOURCE_DIR}/../../Inc")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

file(GLOB BZIP2_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# 정적 라이브러리
add_library(bz2 STATIC ${BZIP2_SOURCES})

target_include_directories(bz2
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

if(UNIX)
    target_compile_options(bz2 PRIVATE
        -O2
        -fPIC
        -Wall
    )
endif()

set_target_properties(bz2 PROPERTIES
    OUTPUT_NAME "bz2"
)

# 헤더 복사
add_custom_command(TARGET bz2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${INC_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/bzlib.h
        ${INC_DIR}/bzlib.h
)