################################################################################
# Test Cases
################################################################################

file(GLOB TEST_SRC
    "*.cpp"
    "acceptance/*.cpp"
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
