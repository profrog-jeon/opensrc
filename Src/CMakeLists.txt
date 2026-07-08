cmake_minimum_required(VERSION 3.2)

set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/../Build/LinuxRelease)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

function(copy_headers SOURCE_DIR DEST_DIR TARGET_NAME)
    file(GLOB HEADER_FILES CONFIGURE_DEPENDS "${SOURCE_DIR}/*.h")
    add_custom_target(${TARGET_NAME} ALL)
    foreach(header ${HEADER_FILES})
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${DEST_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${header}"
                "${DEST_DIR}"
        )
    endforeach()
endfunction()

#add_subdirectory(sqlite3Lib)
#add_subdirectory(pcre2)
#add_subdirectory(bzip2)
#add_subdirectory(unegg)
#add_custom_target(liblzma ALL
#    COMMAND ./configure
#        --disable-shared
#        --enable-static
#        --disable-xz
#        --disable-xzdec
#        --disable-lzmadec
#        --disable-lzmainfo
#    COMMAND make -j8
#	COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        ${CMAKE_SOURCE_DIR}/liblzma/src/liblzma/.libs/liblzma.a
#        ${OUTPUT_DIR}/liblzma.a
#	COMMAND ${CMAKE_COMMAND} -E copy_directory
#		${CMAKE_SOURCE_DIR}/liblzma/src/liblzma/api
#		${CMAKE_SOURCE_DIR}/../Inc/lzma
#    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/liblzma
#)

#if(NOT EXISTS "${CMAKE_SOURCE_DIR}/zstd/CMakeLists.txt")
#    execute_process(
#        COMMAND ${CMAKE_COMMAND} -E tar xf "${CMAKE_SOURCE_DIR}/zstd.zip"
#        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
#        RESULT_VARIABLE ret
#    )
#endif()
#set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
#set(ZSTD_BUILD_TESTS OFF CACHE BOOL "" FORCE)
#set(ZSTD_BUILD_SHARED OFF CACHE BOOL "" FORCE)
#set(ZSTD_BUILD_STATIC ON CACHE BOOL "" FORCE)
#add_subdirectory(zstd)
#copy_headers(
#    ${CMAKE_SOURCE_DIR}/zstd/lib
#    ${CMAKE_SOURCE_DIR}/../Inc
#    copy_zstd_headers
#)

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/lz4-1.10.0/CMakeLists.txt")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xf "${CMAKE_SOURCE_DIR}/lz4-1.10.0.zip"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        RESULT_VARIABLE ret
    )
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(LZ4_BUILD_CLI OFF CACHE BOOL "" FORCE)
set(LZ4_BUILD_LEGACY_LZ4C OFF CACHE BOOL "" FORCE)
set(LZ4_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(lz4-1.10.0/build/cmake)

# 헤더 복사
copy_headers(
    ${CMAKE_SOURCE_DIR}/lz4-1.10.0/lib
    ${CMAKE_SOURCE_DIR}/../Inc
    copy_lz4_headers
)