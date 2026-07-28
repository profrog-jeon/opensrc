cmake_minimum_required(VERSION 3.1.0)

set(TARGET_NAME unegg)
set(PLATFORM_DIRECTORY "")
set(PLATFORM_FILES "")
set(PLATFORM_DEFINITION "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wl,-rpath,'$ORIGIN'")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../Build/LinuxRelease)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../Build/LinuxRelease)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../Build/LinuxRelease)
message(STATUS "CMAKE_ARCHIVE_OUTPUT_DIRECTORY: " ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
message(STATUS "CMAKE_LIBRARY_OUTPUT_DIRECTORY: " ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
message(STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY: " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

##############################################################
## Utility functions
##############################################################
function(exclude_from_list SOURCES_LIST EXPRESSION)
	file(GLOB_RECURSE EXCLUDE ${EXPRESSION})	
	if(EXCLUDE)
		list(REMOVE_ITEM ${SOURCES_LIST} ${EXCLUDE})
	endif()
	set(${SOURCES_LIST} ${${SOURCES_LIST}} PARENT_SCOPE)
endfunction(exclude_from_list)

function(include_to_list SOURCES_LIST EXPRESSION)
	file(GLOB_RECURSE EXCLUDE ${EXPRESSION})	
	if(EXCLUDE)
		list(APPEND ${SOURCES_LIST} ${EXCLUDE})
	endif()
	set(${SOURCES_LIST} ${${SOURCES_LIST}} PARENT_SCOPE)
endfunction(include_to_list)

##############################################################
## Source Filtering
##############################################################
file(GLOB_RECURSE SOURCES *.cpp *.c)
exclude_from_list(SOURCES *Test.cpp)
exclude_from_list(SOURCES cpu_info_arm.c)
exclude_from_list(SOURCES Interfaces.cpp)

foreach(SOURCE IN LISTS SOURCES)
	message(STATUS "SOURCE - " ${SOURCE})
endforeach(SOURCE)

add_definitions(-D_7ZIP_ST -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_REENTRANT -DENV_UNIX -DBREAK_HANDLER -DUNIX_USE_WIN_FILE
    -DNO_PCLMUL
    -DNO_XOP
    -DNO_AVX2
    -DNO_SSE2
    ${PLATFORM_DEFINITION}
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

add_library(${TARGET_NAME} STATIC ${SOURCES})
target_link_libraries(${TARGET_NAME} -static-libgcc -static-libstdc++ -static)
