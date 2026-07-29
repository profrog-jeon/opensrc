cmake_minimum_required(VERSION 3.2)

set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/../Build/LinuxRelease)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

function(extract_zip ZIPFILE DIRNAME)
	message(STATUS "Extracting ${ZIPFILE}")
	execute_process(
		COMMAND ${CMAKE_COMMAND} -E tar xf "${CMAKE_SOURCE_DIR}/${ZIPFILE}"
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
		RESULT_VARIABLE ret
	)
	if(NOT ret EQUAL 0)
		message(FATAL_ERROR "Failed to extract ${ZIPFILE}")
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

set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "" FORCE)

add_subdirectory(sqlite3Lib)
add_subdirectory(pcre2)
add_subdirectory(unegg)
add_subdirectory(bzip2)
copy_headers2(${CMAKE_SOURCE_DIR}/bzip2 ${CMAKE_SOURCE_DIR}/../Inc copy_bzip2_headers)


extract_zip(liblzma-master.zip liblzma-master)
set(LZMA_DIR "${CMAKE_SOURCE_DIR}/liblzma-master")
add_custom_target(liblzma ALL
    COMMAND ./configure
        --disable-shared
        --enable-static
        --with-pic
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

extract_zip(yara-4.5.8.zip yara-4.5.8)
set(YARA_DIR "${CMAKE_SOURCE_DIR}/yara-4.5.8")
add_custom_target(yara ALL
    COMMAND ./build.sh
        --disable-shared
        --enable-static
		--with-pic
        --without-crypto
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${YARA_DIR}/.libs/libyara.a
        ${OUTPUT_DIR}/libyara.a
    WORKING_DIRECTORY ${YARA_DIR}
)
copy_headers(
    ${YARA_DIR}/libyara/include
    ${CMAKE_SOURCE_DIR}/../Inc
    yara
)
