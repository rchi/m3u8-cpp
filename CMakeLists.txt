cmake_minimum_required (VERSION 3.0)
project(M3U8)
set(CMAKE_VERBOSE_MAKEFILE ON)

include_directories(
    include
)

ADD_SUBDIRECTORY(include)
ADD_SUBDIRECTORY(example)
ADD_SUBDIRECTORY(test)
