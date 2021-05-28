

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
include(${CMAKE_CURRENT_LIST_DIR}/dependencies/openssl.cmake)
endif()
include(${CMAKE_CURRENT_LIST_DIR}/dependencies/boost.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/dependencies/json.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/dependencies/url.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/dependencies/cereal.cmake)
