set(BOOST_VERSION 1.72.0)
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