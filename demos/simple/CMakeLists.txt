add_executable(simple
    simple.cpp
    ${CEF_CMAKE_EXECUTABLE_RESOURCES}
)

target_link_libraries(simple
    cefdll_wrapper
)
