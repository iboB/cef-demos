set(srcs)
src_group(resource srcs
    common/Resources.hpp
    common/Resources.cpp
)

src_group(platform srcs
    common/PlatformUtil.hpp
    common/PlatformUtil.cpp
)

add_library(demos-common STATIC
    ${srcs}
)

target_include_directories(demos-common PUBLIC .)

target_link_libraries(demos-common PUBLIC
    cefdll_wrapper
)
