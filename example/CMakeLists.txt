################################################################################
# Examples
################################################################################
set(CMAKE_CXX_STANDARD 17)

option (WITH_CURL "Option with libcurl" ON)
option (WITH_FFMPEG "Option with FFMPEG" ON)
option (WITH_ASAN "Option with ASAN" OFF)

IF(WITH_CURL)
    find_package(CURL REQUIRED)
ENDIF(WITH_CURL)

IF(WITH_FFMPEG)
    PKG_CHECK_MODULES(SDL2 "sdl2")
    PKG_CHECK_MODULES(LIBAVFORMAT "libavformat")
    PKG_CHECK_MODULES(LIBAVCODEC "libavcodec")
    PKG_CHECK_MODULES(LIBAVUTIL "libavutil")
    PKG_CHECK_MODULES(LIBSWSCALE "libswscale")
ENDIF(WITH_FFMPEG)

IF(WITH_ASAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -O0")
ENDIF(WITH_ASAN)

################################################################################

add_executable(
    m3u8-file
    file.cpp
)

################################################################################

IF(WITH_CURL)
    add_executable(
        m3u8-url
        url.cpp
    )

    target_link_libraries(
        m3u8-url
        ${CURL_LIBRARIES}
        -pthread
    )
ENDIF(WITH_CURL)

################################################################################

IF(WITH_CURL)
    add_executable(
        m3u8-download
        download.cpp
    )

    target_link_libraries(
        m3u8-download
        ${CURL_LIBRARIES}
        -pthread
    )
ENDIF(WITH_CURL)

################################################################################

IF(WITH_FFMPEG AND WITH_CURL)
    add_executable(
        m3u8-player
        ffmpeg.cpp
    )

    target_link_libraries(
        m3u8-player
        ${CURL_LIBRARIES}
        ${LIBAVFORMAT_LIBRARIES}
        ${LIBAVCODEC_LIBRARIES}
        ${LIBAVUTIL_LIBRARIES}
        ${LIBSWSCALE_LIBRARIES}
        ${SDL2_LIBRARIES}
        -pthread
    )
ENDIF(WITH_FFMPEG AND WITH_CURL)
