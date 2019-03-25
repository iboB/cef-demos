add_executable(simple-headless
    simple-headless.cpp
    ${CEF_CMAKE_EXECUTABLE_RESOURCES}
)

target_link_libraries(simple-headless
    cefdll_wrapper
)
