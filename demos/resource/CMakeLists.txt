add_executable(resource
    main.cpp
    ${CEF_CMAKE_EXECUTABLE_RESOURCES}
)

target_link_libraries(resource
    demos-common
)
