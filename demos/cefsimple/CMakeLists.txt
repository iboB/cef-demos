set(srcs)

src_group(sources srcs
    simple_app.cc
    simple_app.h
    simple_handler.cc
    simple_handler.h
)

if(CEFDEMOS_OS_LINUX)
    src_group(sources srcs
        cefsimple_linux.cc
        simple_handler_linux.cc
    )
elseif(CEFDEMOS_OS_MACOSX)
    src_group(sources srcs
        cefsimple_mac.mm
        simple_handler_mac.mm
    )
elseif(CEFDEMOS_OS_WINDOWS)
    src_group(sources srcs
        cefsimple_win.cc
        simple_handler_win.cc
    )
endif()

add_executable(cefsimple
    ${srcs}
    ${CEF_CMAKE_EXECUTABLE_RESOURCES}
)

if(CEFDEMOS_OS_WINDOWS)
    set_target_properties(cefsimple PROPERTIES WIN32_EXECUTABLE 1)
endif()

target_link_libraries(cefsimple
    cefdll_wrapper
)

