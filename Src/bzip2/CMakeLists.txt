cmake_minimum_required(VERSION 3.12)

project(bzip2 C)

set(CMAKE_C_STANDARD 99)

file(GLOB BZIP2_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

# 정적 라이브러리
add_library(bz2 STATIC ${BZIP2_SOURCES})
target_include_directories(bz2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(UNIX)
    target_compile_options(bz2 PRIVATE -O2 -fPIC -Wall)
endif()

set_target_properties(bz2 PROPERTIES OUTPUT_NAME "bz2")