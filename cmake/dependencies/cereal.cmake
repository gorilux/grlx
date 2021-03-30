include(ExternalProject)

# gcc 8 spits out warnings from Boost.Mpl about unnecessary parentheses
# https://github.com/CauldronDevelopmentLLC/cbang/issues/26
# (this library bundles Boost)
# TODO: Perhaps do a check for Boost and gcc version before adding this flag?
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-parentheses")

# set(URI_FILENAME
#     "${CMAKE_CURRENT_BINARY_DIR}/uri/src/uri-build/src/${CMAKE_STATIC_LIBRARY_PREFIX}network-uri${CMAKE_STATIC_LIBRARY_SUFFIX}"
# )

externalproject_add(cereal
    GIT_REPOSITORY https://github.com/USCiLab/cereal
    GIT_TAG master
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
    CMAKE_ARGS
        -DTHREAD_SAFE=ON
        -DSKIP_PORTABILITY_TEST=ON
        -DWITH_WERROR=OFF
        -DJUST_INSTALL_CEREAL=ON
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
        -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
        -DANDROID_ABI=${ANDROID_ABI}
        -DANDROID_PLATFORM=${ANDROID_PLATFORM}
    PREFIX "cereal"
)

add_library(lib_cereal INTERFACE)
add_dependencies(lib_cereal cereal)
add_library(lib::cereal ALIAS lib_cereal)

target_include_directories(lib_cereal
    INTERFACE
        "${CMAKE_CURRENT_BINARY_DIR}/cereal/src/cereal/include"
)
# target_link_libraries(lib_cereal
#     INTERFACE ${URI_FILENAME}
# )


