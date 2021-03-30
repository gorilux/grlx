include(FetchContent)




# FetchContent_GetProperties(range-v3)
# if(NOT range-v3_POPULATED)

#   FetchContent_Populate(range-v3)
#   add_subdirectory(${range-v3_SOURCE_DIR} ${range-v3_BINARY_DIR})
# endif()

# set(range-v3_DIR ${range-v3_BINARY_DIR})

# FetchContent_MakeAvailable(range-v3)

FetchContent_Declare(
  skyr_url
  GIT_REPOSITORY https://github.com/cpp-netlib/url.git
  GIT_TAG main
  PATCH_COMMAND git apply --ignore-space-change --ignore-whitespace "${CMAKE_CURRENT_SOURCE_DIR}/cmake/dependencies/skyr_url.patch" || true
)


 FetchContent_GetProperties(skyr_url)
 if(NOT skyr_url_POPULATED)

   FetchContent_Populate(skyr_url)
   add_subdirectory(${skyr_url_SOURCE_DIR} ${skyr_url_BINARY_DIR})
#   #add_subdirectory(${fmt_SOURCE_DIR} EXCLUDE_FROM_ALL)
  
  
endif()


# FetchContent_MakeAvailable(skyr_url)