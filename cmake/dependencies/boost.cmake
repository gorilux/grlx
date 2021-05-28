
set(BOOST_VERSION 1.72.0)
if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
set(BOOST_COMPONENTS
    context
    coroutine
    date_time
    filesystem
    iostreams
    program_options
    regex
    system
    unit_test_framework
)
find_package(Boost ${BOOST_VERSION} REQUIRED COMPONENTS ${BOOST_COMPONENTS})
find_package(Threads REQUIRED)
elseif()
set(BOOST_COMPONENTS
    context
    coroutine
    date_time
    filesystem
    iostreams
    program_options
    regex
    system
    unit_test_framework
)

set_target_properties(Boost PROPERTIES COMPILE_FLAGS "-s USE_BOOST_HEADERS=1 -s ERROR_ON_UNDEFINED_SYMBOLS=0" LINK_FLAGS "-s USE_BOOST_HEADERS=1 -s ERROR_ON_UNDEFINED_SYMBOLS=0")
endif()



