cmake_minimum_required(VERSION 3.2)

set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/../Build/LinuxRelease)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

function(extract_zip ZIPFILE DIRNAME)
	message(STATUS "Extracting ${ZIPFILE}")
	if(EXISTS "${CMAKE_SOURCE_DIR}/${DIRNAME}")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E rm -fR "${CMAKE_SOURCE_DIR}/${DIRNAME}"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            RESULT_VARIABLE ret
        )
	endif()
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

extract_zip(yara-4.5.8.zip yara-4.5.8)
set(YARA_DIR "${CMAKE_SOURCE_DIR}/yara-4.5.8")
add_custom_target(yara ALL
    COMMAND ./bootstrap.sh
    COMMAND ./configure
        --disable-shared
        --enable-static
        --with-pic
        --without-crypto
    COMMAND make -j8
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
