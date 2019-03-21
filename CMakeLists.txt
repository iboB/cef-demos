cmake_minimum_required(VERSION 3.10)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(cef-demo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/cef-cmake/cmake
)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set(CEFDEMOS_OS_MACOSX 1)
    set(CEFDEMOS_OS_POSIX 1)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    set(CEFDEMOS_OS_LINUX 1)
    set(CEFDEMOS_OS_POSIX 1)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    set(CEFDEMOS_OS_WINDOWS 1)
else()
    message(FATAL_ERROR "CEF-Demos: Unsupported target platform")
endif()

include(src_group)

# configure
set(CEF_USE_SANDBOX OFF CACHE BOOL "Force turning off of sandbox")
include(cef_cmake)
add_subdirectory(cef-cmake)

add_subdirectory(demos)
