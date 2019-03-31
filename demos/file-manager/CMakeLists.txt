add_executable(file-manager
    file-manager-main.cpp
    FileManager.cpp
    FileManager.hpp
    ${CEF_CMAKE_EXECUTABLE_RESOURCES}
)

target_link_libraries(file-manager
    demos-common
)

if(MSVC)
    target_compile_definitions(file-manager PRIVATE
        -D_CRT_SECURE_NO_WARNINGS
    )
endif()
