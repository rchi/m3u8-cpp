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
