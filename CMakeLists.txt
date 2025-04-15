cmake_minimum_required (VERSION 4.0)
project(M3U8)
set(CMAKE_VERBOSE_MAKEFILE ON)

add_library(sdl2lib SHARED IMPORTED)
set_property(TARGET sdl2lib PROPERTY IMPORTED_LOCATION "/opt/homebrew/lib/libSDL2-2.0.0.dylib")
add_library(avcodeclib SHARED IMPORTED)
set_property(TARGET avcodeclib PROPERTY IMPORTED_LOCATION "/opt/homebrew/lib/libavcodec.dylib")
add_library(avformatlib SHARED IMPORTED)
set_property(TARGET avformatlib PROPERTY IMPORTED_LOCATION "/opt/homebrew/lib/libavformat.dylib")
add_library(swscalelib SHARED IMPORTED)
set_property(TARGET swscalelib PROPERTY IMPORTED_LOCATION "/opt/homebrew/lib/libswscale.dylib")

include_directories(
    include
    /opt/homebrew/include
)

target_link_libraries(sdl2lib)
link_libraries(sdl2lib avcodeclib avformatlib swscalelib)

ADD_SUBDIRECTORY(include)
ADD_SUBDIRECTORY(example)
ADD_SUBDIRECTORY(test)
