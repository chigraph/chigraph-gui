cmake_minimum_required(VERSION 3.0)

set(BUILD_EXAMPLES OFF CACHE BOOL "")
add_subdirectory(third_party/nodeeditor)

add_subdirectory(chigraph)
add_subdirectory(src)
