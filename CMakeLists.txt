cmake_minimum_required (VERSION 3.15)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules/")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")


project(grlx LANGUAGES CXX C)
include(FetchContent)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
SET(GRLX_COMPILE_OPTIONS  "-fcoroutines" )
endif()

add_compile_options("-Wall")
add_compile_options(${GRLX_COMPILE_OPTIONS})
include(GrlxDependencies)

add_library(grlx_base INTERFACE)
add_library(grlx::base ALIAS grlx_base)

# add_definitions(-DBOOST_ASIO_DISABLE_CONCEPTS)

target_compile_options(grlx_base 
    INTERFACE ${GRLX_COMPILE_OPTIONS}
)

target_compile_definitions(grlx_base INTERFACE BOOST_BEAST_USE_STD_STRING_VIEW)

target_include_directories( grlx_base 
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
target_link_libraries( grlx_base INTERFACE 
    lib::cereal
#    Boost::asio
    Boost::boost
#    OpenSSL::headers
    Threads::Threads
    Boost::boost
    Boost::filesystem 
    Boost::regex     
    OpenSSL::SSL 
    OpenSSL::Crypto    
    nlohmann_json
    skyr::skyr-url 
)
endif()

# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include)




file(GLOB_RECURSE lib_grlx_service_sources "lib/service/*.cpp" "./grlx/service/*.h"  "./include/service/*.hpp")

list(APPEND lib_grlx_sources ${lib_grlx_service_sources})

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
file(GLOB_RECURSE lib_grlx_utility_sources "lib/utility/*.cpp" "grlx/utility/*.h" "grlx/utility/*.hpp")
file(GLOB_RECURSE lib_grlx_net_sources "lib/net/*.cpp" "./grlx/net/*.h"  "./grlx/net/*.hpp")
list(APPEND lib_grlx_sources ${lib_grlx_net_sources})
list(APPEND lib_grlx_sources ${lib_grlx_utility_sources})
endif()


add_library(grlx STATIC ${lib_grlx_sources})

target_link_libraries(grlx grlx::base)

add_library(lib::grlx ALIAS grlx)

#add_subdirectory(src)

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
enable_testing()
add_subdirectory(test)
endif()



#add_library()


