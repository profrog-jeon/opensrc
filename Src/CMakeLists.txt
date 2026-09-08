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
    file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "${SOURCE_DIR}/*.h" "${SOURCE_DIR}/*.hpp")
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
    file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "${SOURCE_DIR}/*.h" "${SOURCE_DIR}/*.hpp")
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

extract_zip(jansson-2.15.1.zip jansson-2.15.1)
set(JANSSON_DIR "${CMAKE_SOURCE_DIR}/jansson-2.15.1")
add_custom_target(jansson ALL
    COMMAND autoreconf -fi
    COMMAND ./configure
        --disable-shared
        --enable-static
        --with-pic
    COMMAND make -j8
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${JANSSON_DIR}/src/.libs/libjansson.a
        ${OUTPUT_DIR}/libjansson.a
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/../Inc/jansson-linux"
    COMMAND ${CMAKE_COMMAND} -E rename
        ${JANSSON_DIR}/src/jansson.h
        ${CMAKE_SOURCE_DIR}/../Inc/jansson-linux/jansson.h
    COMMAND ${CMAKE_COMMAND} -E rename
        ${JANSSON_DIR}/src/jansson_config.h
        ${CMAKE_SOURCE_DIR}/../Inc/jansson-linux/jansson_config.h
    WORKING_DIRECTORY ${JANSSON_DIR}
)

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

extract_zip(minizip-ng-develop.zip minizip-ng-develop)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(MZ_ZLIB ON CACHE BOOL "" FORCE)
set(MZ_BZIP2 OFF CACHE BOOL "" FORCE)
set(MZ_LZMA OFF CACHE BOOL "" FORCE)
set(MZ_ZSTD OFF CACHE BOOL "" FORCE)
set(MZ_PPMD OFF CACHE BOOL "" FORCE)
set(MZ_OPENSSL ON CACHE BOOL "" FORCE)
# Point find_package(OpenSSL) at the OpenSSL already built into this project
# instead of a system install (Inc/openssl headers, Build/LinuxRelease static libs).
set(OPENSSL_USE_STATIC_LIBS ON)
set(OPENSSL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/../Inc CACHE PATH "" FORCE)
set(OPENSSL_CRYPTO_LIBRARY ${OUTPUT_DIR}/libcrypto.a CACHE FILEPATH "" FORCE)
set(OPENSSL_SSL_LIBRARY ${OUTPUT_DIR}/libssl.a CACHE FILEPATH "" FORCE)
add_subdirectory(minizip-ng-develop)
copy_headers2(
    ${CMAKE_SOURCE_DIR}/minizip-ng-develop
    ${CMAKE_SOURCE_DIR}/../Inc/minizip
    copy_minizip_headers
)

extract_zip(opencv-4.12.0.zip opencv-4.12.0)
set(OPENCV_DIR "${CMAKE_SOURCE_DIR}/opencv-4.12.0")

extract_zip(opencv_contrib-4.12.0.zip opencv_contrib-4.12.0)
set(OPENCV_CONTRIB_DIR "${CMAKE_SOURCE_DIR}/opencv_contrib-4.12.0")
set(OPENCV_EXTRA_MODULES_PATH "${OPENCV_CONTRIB_DIR}/modules" CACHE PATH "" FORCE)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
# QR 코드 디코딩(objdetect의 QRCodeDetector, wechat_qrcode)에 필요한 모듈만 빌드한다.
# objdetect -> calib3d -> features2d, flann 이 강제 의존성이고, wechat_qrcode는
# opencv_contrib 모듈로 opencv_dnn에 의존한다.
set(BUILD_LIST "core,imgproc,imgcodecs,objdetect,calib3d,features2d,flann,dnn,wechat_qrcode,highgui" CACHE STRING "" FORCE)
set(BUILD_opencv_wechat_qrcode ON CACHE BOOL "" FORCE)
set(BUILD_opencv_dnn ON CACHE BOOL "" FORCE)
set(BUILD_opencv_world OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_apps OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_PERF_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(BUILD_JAVA OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_python2 OFF CACHE BOOL "" FORCE)
set(BUILD_opencv_python3 OFF CACHE BOOL "" FORCE)
# 주요 이미지 포맷(PNG/JPEG/TIFF/WEBP/JPEG2000)은 켜고, GUI/비디오 등
# QR 디코딩과 무관한 백엔드 의존성만 끈다. BMP/GIF는 별도 라이브러리 없이
# imgcodecs 자체에 내장되어 있어 항상 지원된다.
set(WITH_PNG ON CACHE BOOL "" FORCE)
set(WITH_JPEG ON CACHE BOOL "" FORCE)
set(WITH_TIFF ON CACHE BOOL "" FORCE)
set(WITH_WEBP ON CACHE BOOL "" FORCE)
set(WITH_OPENJPEG ON CACHE BOOL "" FORCE)
set(WITH_OPENEXR OFF CACHE BOOL "" FORCE)
set(WITH_JASPER OFF CACHE BOOL "" FORCE)
set(WITH_FFMPEG OFF CACHE BOOL "" FORCE)
set(WITH_GTK OFF CACHE BOOL "" FORCE)
set(WITH_QT OFF CACHE BOOL "" FORCE)
set(WITH_V4L OFF CACHE BOOL "" FORCE)
set(WITH_1394 OFF CACHE BOOL "" FORCE)
set(WITH_CUDA OFF CACHE BOOL "" FORCE)
set(WITH_OPENCL OFF CACHE BOOL "" FORCE)
set(WITH_TBB OFF CACHE BOOL "" FORCE)
set(WITH_IPP OFF CACHE BOOL "" FORCE)
set(WITH_ITT OFF CACHE BOOL "" FORCE)
# dnn / wechat_qrcode가 caffe 모델(prototxt+caffemodel)을 읽는 데 필요하다.
set(WITH_PROTOBUF ON CACHE BOOL "" FORCE)
add_subdirectory(opencv-4.12.0)

# OpenCV는 CMAKE_ARCHIVE_OUTPUT_DIRECTORY를 자체적으로
# ${CMAKE_CURRENT_BINARY_DIR}/lib, .../3rdparty/lib 로 덮어써서 라이브러리가
# OUTPUT_DIR이 아니라 빌드 디렉터리 안에 그대로 남는다. 모든 대상 라이브러리가
# 만들어진 뒤 OUTPUT_DIR로 복사해준다.
add_custom_target(copy_opencv_libs ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_BINARY_DIR}/opencv-4.12.0/lib ${OUTPUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_BINARY_DIR}/opencv-4.12.0/3rdparty/lib ${OUTPUT_DIR}
)
add_dependencies(copy_opencv_libs
    opencv_core opencv_imgproc opencv_imgcodecs opencv_flann
    opencv_features2d opencv_calib3d opencv_objdetect
    opencv_dnn opencv_wechat_qrcode opencv_highgui
    libpng libtiff libwebp libopenjp2 libjpeg-turbo libprotobuf
)

# 공개 헤더는 모듈별 include 디렉터리와, CMake가 빌드 시점에 생성하는
# opencv2/opencv_modules.hpp 등 두 군데에 나뉘어 있어 각각 복사한다.
copy_headers2(
    ${OPENCV_DIR}/include
    ${CMAKE_SOURCE_DIR}/../Inc
    copy_opencv_umbrella_headers
)
foreach(OPENCV_MOD core imgproc imgcodecs flann features2d calib3d objdetect dnn highgui)
    copy_headers2(
        ${OPENCV_DIR}/modules/${OPENCV_MOD}/include
        ${CMAKE_SOURCE_DIR}/../Inc
        copy_opencv_${OPENCV_MOD}_headers
    )
endforeach()
# wechat_qrcode는 opencv_contrib 쪽에 있어 소스 위치가 다르다.
copy_headers2(
    ${OPENCV_CONTRIB_DIR}/modules/wechat_qrcode/include
    ${CMAKE_SOURCE_DIR}/../Inc
    copy_opencv_wechat_qrcode_headers
)
copy_headers2(
    ${CMAKE_BINARY_DIR}/opencv2
    ${CMAKE_SOURCE_DIR}/../Inc/opencv2
    copy_opencv_generated_headers
)
