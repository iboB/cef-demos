add_executable(resource
    resource.cpp
    ${CEF_CMAKE_EXECUTABLE_RESOURCES}
)

target_link_libraries(resource
    demos-common
)
