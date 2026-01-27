cmake_minimum_required(VERSION 3.2)
set(TARGET_NAME pcre2Test)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wno-narrowing")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(APPLE)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../Build/MacOSXRelease)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../Build/MacOSXRelease)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../Build/MacOSXRelease)
else()
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../Build/Test)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../Build/Test)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../Build/Test)
endif()

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
	file(GLOB_RECURSE INCLUDES ${EXPRESSION})	
	if(INCLUDES)
		list(APPEND ${SOURCES_LIST} ${INCLUDES})
	endif()
	set(${SOURCES_LIST} ${${SOURCES_LIST}} PARENT_SCOPE)
endfunction(include_to_list)

##############################################################
## Source Filtering
##############################################################
file(GLOB_RECURSE SOURCES *.c *.cpp)	

if(APPLE)
	exclude_from_list(SOURCES *_Win.cpp)
	exclude_from_list(SOURCES *_Linux.cpp)
	include_to_list(SOURCES *_Mac_Linux.cpp)
else()
	exclude_from_list(SOURCES *_Win.cpp)
	exclude_from_list(SOURCES *_Mac.cpp)
	include_to_list(SOURCES *_Linux_Mac.cpp)
endif()

foreach(SOURCE IN LISTS SOURCES)
	message(STATUS "SOURCE - " ${SOURCE})
endforeach(SOURCE)

##############################################################
## Build setting
##############################################################
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../opensrc/Inc)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../cppcore/Build/LinuxRelease)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Build/LinuxRelease)
add_executable(${TARGET_NAME} ${SOURCES})
target_compile_definitions(${TARGET_NAME} PUBLIC -DUNICODE16 -D_UNICODE16 -DHAVE_CONFIG_H)
target_link_libraries(${TARGET_NAME} cppcore pcre2 minizip z dl pthread)