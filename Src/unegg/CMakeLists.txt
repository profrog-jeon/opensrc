cmake_minimum_required(VERSION 2.8)
set(TARGET_NAME unegg)

##############################################################
## Declaration of output path
##############################################################
if(APPLE)
	set(OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../Build/MacOSXRelease)
else()
	set(OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../Build/LinuxRelease)
endif()
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
message(STATUS "OUTPUT_DIRECTORY: " ${OUTPUT_DIRECTORY})
message(STATUS "CMAKE_PROJECT_NAME: " ${CMAKE_PROJECT_NAME})
message(STATUS "TARGET_NAME: " ${TARGET_NAME})


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
	file(GLOB_RECURSE INCLUDE ${EXPRESSION})	
	if(INCLUDE)
		list(APPEND ${SOURCES_LIST} ${INCLUDE})
	endif()
	set(${SOURCES_LIST} ${${SOURCES_LIST}} PARENT_SCOPE)
endfunction(include_to_list)


##############################################################
## Sources Filtering
##############################################################
file(GLOB_RECURSE SOURCES *.cpp)
include_to_list(SOURCES *.c)

##############################################################
## Build setting
##############################################################
if(APPLE)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR})
	include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../../cppcore/Inc")
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../cppcore/Build/MacOSXRelease)
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Build/x64MacOSXRelease)
	add_executable(${TARGET_NAME} ${SOURCES})
	target_link_libraries(${TARGET_NAME} cppcore iconv)
else()
	include_directories(${CMAKE_CURRENT_SOURCE_DIR})
	include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../../cppcore/Inc")
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../cppcore/Build/LinuxRelease)
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../Build/x64LinuxRelease)
	add_executable(${TARGET_NAME} ${SOURCES})
	target_link_libraries(${TARGET_NAME} cppcore pthread dl rt)
endif()
