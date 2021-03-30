
find_package(OpenSSL REQUIRED)

# Some container classes rely on openssl headers, but not on linking to
# the library proper.
if (NOT TARGET OpenSSL::headers)
    add_library(openssl_headers INTERFACE)
    add_library(OpenSSL::headers ALIAS openssl_headers)
    target_include_directories(openssl_headers
        INTERFACE "${OPENSSL_INCLUDE_DIR}"
    )
    if (TARGET built_openssl)
        add_dependencies(openssl_headers built_openssl)
    endif()
endif()


