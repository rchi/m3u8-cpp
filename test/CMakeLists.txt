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

add_library(gtestlib SHARED IMPORTED)
set_property(TARGET gtestlib PROPERTY IMPORTED_LOCATION "/opt/homebrew/lib/libgtest.a")

target_link_libraries(
    m3u8-test
    gtestlib
    ${CURL_LIBRARIES}
    -pthread
)
