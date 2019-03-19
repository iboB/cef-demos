set(res)
if(MSVC)
    src_group(Resources res
        resources/win/cef.exe.manifest
        resources/win/compatibility.manifest
    )
endif()

add_executable(simple
    simple.cpp
    ${res}
)

target_link_libraries(simple
    cefdll_wrapper
)
