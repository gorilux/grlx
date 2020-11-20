
project(grlx)

set( GRLX_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/grlx/include" )

include_directories(
  ${GRLX_INCLUDE}
  ${GRLX_INCLUDE}/grlx/service
  
)


add_definitions(-DBOOST_ASIO_DISABLE_CONCEPTS)

# file(GLOB_RECURSE GRLX_SOURCES "*.cpp")
set( GRLX_SOURCES 
  grlx/cpp/service/servicecontainer.cpp
)

# add_library(grlx STATIC grlx/cpp/servicecontainer.cpp )
add_library(grlx STATIC ${GRLX_SOURCES})

#add_definitions(-DZMQ_BUILD_DRAFT_API)

#target_include_directories ( grlx PUBLIC ${GRLX_INCLUDE} ${GRLX_RAPIDJSON_INCLUDE})
target_include_directories ( grlx INTERFACE
                            $<BUILD_INTERFACE:${GRLX_INCLUDE}>)
set_target_properties(grlx PROPERTIES COMPILE_FLAGS -DBOOST_ASIO_DISABLE_CONCEPTS)         
target_link_libraries(grlx)
add_dependencies( grlx  OpenSSL::SSL )
