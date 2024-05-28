cmake_minimum_required (VERSION 3.0)
project(M3U8)
set(CMAKE_VERBOSE_MAKEFILE ON)

################################################################################
# Common
################################################################################

option (WITH_CURL "Option with libcurl" ON)

include_directories(
    include
)

################################################################################
# Install
################################################################################

include(GNUInstallDirs)
set(M3U8_INCLUDE_BUILD_DIR          "${PROJECT_SOURCE_DIR}/include/")
set(M3U8_INCLUDE_INSTALL_DIR        "${CMAKE_INSTALL_INCLUDEDIR}")
install(
    DIRECTORY ${M3U8_INCLUDE_BUILD_DIR}
    DESTINATION ${M3U8_INCLUDE_INSTALL_DIR}
)

################################################################################
# Examples
################################################################################

add_executable(
    m3u8-file
    example/file.cpp
)

IF(WITH_CURL)
    find_package(CURL REQUIRED)
    add_executable(
        m3u8-url
        example/url.cpp
    )

    target_link_libraries(
        m3u8-url
        ${CURL_LIBRARIES}
        -pthread
    )
ENDIF(WITH_CURL)

################################################################################
# Test Cases
################################################################################

file(GLOB TEST_SRC
    "test/*.cpp"
    "test/acceptance/*.cpp"
)

add_executable(
    m3u8-test
    ${TEST_SRC}
)

target_link_libraries(
    m3u8-test
    gtest
    ${CURL_LIBRARIES}
    -pthread
)
