cmake_minimum_required(VERSION 3.0)

option(CG_USE_SYSTEM_CHIGRAPH "Try to find chigraph instead of building it here" OFF)

set(BUILD_EXAMPLES OFF CACHE BOOL "")
add_subdirectory(third_party/nodeeditor)

if (CG_USE_SYSTEM_CHIGRAPH)
	find_package(Chigraph REQUIRED)
else()
	add_subdirectory(chigraph)
endif()
add_subdirectory(src)
