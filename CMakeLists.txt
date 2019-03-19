cmake_minimum_required(VERSION 3.10)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(cef-demo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake
)

include(src_group)

# configure
set(CEF_USE_SANDBOX OFF CACHE BOOL "Force turning off of sandbox")
add_subdirectory(cef-cmake)

add_subdirectory(demos)
