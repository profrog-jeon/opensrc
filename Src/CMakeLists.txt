cmake_minimum_required(VERSION 3.2)

set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/../Build/LinuxRelease)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

function(extract_zip ZIPFILE DIRNAME)
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/${DIRNAME}")
        message(STATUS "Extracting ${ZIPFILE}")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xf "${CMAKE_SOURCE_DIR}/${ZIPFILE}"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            RESULT_VARIABLE ret
        )
        if(NOT ret EQUAL 0)
            message(FATAL_ERROR "Failed to extract ${ZIPFILE}")
        endif()
    endif()
endfunction()

function(copy_headers SOURCE_DIR DEST_DIR TARGET)
    file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "${SOURCE_DIR}/*.h")
    foreach(header ${HEADER_FILES})
        file(RELATIVE_PATH REL_PATH "${SOURCE_DIR}" "${header}")	# SOURCE_DIR 기준의 상대 경로
        get_filename_component(REL_DIR "${REL_PATH}" DIRECTORY)		# 상대 경로에서 디렉터리만 추출
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${DEST_DIR}/${REL_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${header}" "${DEST_DIR}/${REL_PATH}"
        )
    endforeach()
endfunction()

function(copy_headers2 SOURCE_DIR DEST_DIR TARGET_NAME)
    file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "${SOURCE_DIR}/*.h")
    add_custom_target(${TARGET_NAME} ALL)
    foreach(header ${HEADER_FILES})
        file(RELATIVE_PATH REL_PATH "${SOURCE_DIR}" "${header}")
        get_filename_component(REL_DIR "${REL_PATH}" DIRECTORY)
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${DEST_DIR}/${REL_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${header}" "${DEST_DIR}/${REL_PATH}")
    endforeach()
endfunction()

add_subdirectory(sqlite3Lib)
add_subdirectory(pcre2)
add_subdirectory(bzip2)
copy_headers2(${CMAKE_SOURCE_DIR}/bzip2 ${CMAKE_SOURCE_DIR}/../Inc copy_bzip2_headers)

add_subdirectory(unegg)

extract_zip(liblzma-master.zip liblzma-master)
set(LZMA_DIR "${CMAKE_SOURCE_DIR}/liblzma-master")
add_custom_target(liblzma ALL
    COMMAND ./configure
        --disable-shared
        --enable-static
        --disable-xz
        --disable-xzdec
        --disable-lzmadec
        --disable-lzmainfo
    COMMAND make -j8
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${LZMA_DIR}/src/liblzma/.libs/liblzma.a
        ${OUTPUT_DIR}/liblzma.a
    WORKING_DIRECTORY ${LZMA_DIR}
)
copy_headers(
    ${LZMA_DIR}/src/liblzma/api
    ${CMAKE_SOURCE_DIR}/../Inc/lzma
    liblzma
)

extract_zip(zstd.zip zstd)
set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_STATIC ON CACHE BOOL "" FORCE)
add_subdirectory(zstd)
copy_headers2(
    ${CMAKE_SOURCE_DIR}/zstd/lib
    ${CMAKE_SOURCE_DIR}/../Inc/zstd
    copy_zstd_headers
)

extract_zip(lz4-1.10.0.zip lz4-1.10.0)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(LZ4_BUILD_CLI OFF CACHE BOOL "" FORCE)
set(LZ4_BUILD_LEGACY_LZ4C OFF CACHE BOOL "" FORCE)
set(LZ4_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(lz4-1.10.0/build/cmake)
copy_headers2(
    ${CMAKE_SOURCE_DIR}/lz4-1.10.0/lib
    ${CMAKE_SOURCE_DIR}/../Inc
    copy_lz4_headers
)

extract_zip(libarchive-3.8.8.zip libarchive-3.8.8)
set(ENABLE_WERROR OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(ENABLE_TAR OFF CACHE BOOL "" FORCE)
set(ENABLE_CPIO OFF CACHE BOOL "" FORCE)
set(ENABLE_CAT OFF CACHE BOOL "" FORCE)
set(ENABLE_UNZIP OFF CACHE BOOL "" FORCE)

set(ENABLE_TEST OFF CACHE BOOL "" FORCE)
set(ENABLE_ZLIB   ON CACHE BOOL "" FORCE)
set(ENABLE_BZip2  ON CACHE BOOL "" FORCE)
set(ENABLE_LIBXML2 OFF CACHE BOOL "" FORCE)
set(ENABLE_EXPAT OFF CACHE BOOL "" FORCE)
set(ENABLE_LZMA ON CACHE BOOL "" FORCE)
set(ENABLE_LZ4 ON CACHE BOOL "" FORCE)
set(ENABLE_ZSTD ON CACHE BOOL "" FORCE)

set(ZLIB_INCLUDE_DIR     "${CMAKE_SOURCE_DIR}/../Inc" CACHE PATH "" FORCE)
set(BZIP2_INCLUDE_DIR    "${CMAKE_SOURCE_DIR}/../Inc" CACHE PATH "" FORCE)
set(LIBLZMA_INCLUDE_DIR  "${CMAKE_SOURCE_DIR}/../Inc" CACHE PATH "" FORCE)
set(LZ4_INCLUDE_DIR      "${CMAKE_SOURCE_DIR}/../Inc" CACHE PATH "" FORCE)
set(ZSTD_INCLUDE_DIR     "${CMAKE_SOURCE_DIR}/../Inc" CACHE PATH "" FORCE)
set(ZLIB_LIBRARY         "${OUTPUT_DIR}/libz.a" CACHE FILEPATH "" FORCE)
set(BZIP2_LIBRARIES      "${OUTPUT_DIR}/libbz2.a" CACHE FILEPATH "" FORCE)
set(LIBLZMA_LIBRARY      "${OUTPUT_DIR}/liblzma.a" CACHE FILEPATH "" FORCE)
set(LZ4_LIBRARY          "${OUTPUT_DIR}/liblz4.a" CACHE FILEPATH "" FORCE)
set(ZSTD_LIBRARY         "${OUTPUT_DIR}/libzstd.a" CACHE FILEPATH "" FORCE)

include_directories(${CMAKE_SOURCE_DIR}/../Inc)
include_directories(${CMAKE_SOURCE_DIR}/../Inc/zstd)
include_directories(${CMAKE_SOURCE_DIR}/../Inc/lzma)
link_directories(${CMAKE_SOURCE_DIR}/../Build/LinuxRelease)

add_subdirectory(libarchive-3.8.8)
copy_headers2(    
    ${CMAKE_SOURCE_DIR}/libarchive-3.8.8/libarchive
    ${CMAKE_SOURCE_DIR}/../Inc/archive
	copy_libarchive_headers
)