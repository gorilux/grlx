cmake_minimum_required(VERSION 3.18)

project(grlx)


file(GLOB_RECURSE GRLX_SOURCES "*.cpp")

# add_library(grlx STATIC grlx/cpp/servicecontainer.cpp )
add_library(grlx STATIC ${GRLX_SOURCES})


#target_include_directories ( grlx PUBLIC ${GRLX_INCLUDE} ${GRLX_RAPIDJSON_INCLUDE})
target_include_directories ( grlx INTERFACE
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/grlx>)
#set_target_properties(grlx PROPERTIES COMPILE_FLAGS -DBOOST_ASIO_DISABLE_CONCEPTS)         
target_link_libraries(grlx)
