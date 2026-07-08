cmake_minimum_required(VERSION 3.2)

set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/../Build/LinuxRelease)

#add_subdirectory(sqlite3Lib)
#add_subdirectory(pcre2)
#add_subdirectory(bzip2)
#add_subdirectory(unegg)
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
        ${CMAKE_SOURCE_DIR}/liblzma/src/liblzma/.libs/liblzma.a
        ${OUTPUT_DIR}/liblzma.a
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/liblzma/src/liblzma/api
		${CMAKE_SOURCE_DIR}/../Inc/lzma
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/liblzma
)
