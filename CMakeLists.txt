cmake_minimum_required(VERSION 3.5)

project(curl-cmake LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT UNIX)
    message(FATAL_ERROR "unsupporsted ${CMAKE_SYSTEM_NAME}")
endif()

if (NOT HAS_THIRDPARTY_OPENSSL)
    message(FATAL_ERROR "you need import 'openssl-cmake' project from github, cmd `git clone xxx`")
else()
    message(STATUS "openssl prefix: ${THIRDPARTY_OPENSSL_PREFIX}")
endif()

set (install_prefix "${CMAKE_CURRENT_BINARY_DIR}/local")

set(THIRDPARTY_CURL_PREFIX ${install_prefix} PARENT_SCOPE)
set(HAS_THIRDPARTY_CURL ON PARENT_SCOPE)

include(ExternalProject)

function(curlMain)

    add_library(thirdparty_curl INTERFACE)
    add_library(thirdparty_curl_static STATIC IMPORTED GLOBAL)
    if (NOT CMD_MAKE)
        message(STATUS "[curl] set CMD_MAKE is make.")
        set(CMD_MAKE make)
    endif()

    message(STATUS "[curl] add external project: curl")

    # 依赖库 https://curl.se/docs/libs.html
    set(configure_command
        "${CMAKE_COMMAND}" -E env "CC=${CMAKE_C_COMPILER}" "AR=${CMAKE_AR}"
        perl ./configure
        "--prefix=${install_prefix}"
        --without-librtmp --without-libidn2 --without-zstd --without-brotli --without-nghttp2 --without-nghttp3 --without-ldap-lib
        "--with-openssl=${THIRDPARTY_OPENSSL_PREFIX}"
        "--with-libssh2=${THIRDPARTY_LIBSSH2L_PREFIX}"
        "CFLAGS=-fPIC"
        "CPPFLAGS=-fPIC"
        )

    ExternalProject_Add(
        curl
        URL "https://github.com/curl/curl/releases/download/curl-7_77_0/curl-7.77.0.tar.gz"
        URL_HASH "SHA256=b0a3428acb60fa59044c4d0baae4e4fc09ae9af1d8a3aa84b2e3fbcd99841f77"
        CONFIGURE_COMMAND ${configure_command}
        BUILD_COMMAND ${CMD_MAKE}
        BUILD_IN_SOURCE 1
        INSTALL_COMMAND ${CMD_MAKE} install
        UPDATE_COMMAND ""
        )
    add_dependencies(curl thirdparty_openssl)
    add_dependencies(thirdparty_curl_static curl)

    set_target_properties(thirdparty_curl_static PROPERTIES IMPORTED_LOCATION
        "${install_prefix}/lib/libcurl.a"
        )

    target_link_libraries(thirdparty_curl INTERFACE thirdparty_curl_static)
    if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
        target_link_libraries(thirdparty_curl INTERFACE "-framework SystemConfiguration" dl z)
    endif()
    target_include_directories(thirdparty_curl INTERFACE "${install_prefix}/include")

endfunction()

function(easyMain)
    add_library(thirdparty_curl_easy
        easy/AutoThreadsafe.cpp
        easy/ftp.cpp
        easy/httpcli.cpp
        )
    target_link_libraries(thirdparty_curl_easy thirdparty_curl thirdparty_openssl thirdparty_libssh2)
    target_include_directories(thirdparty_curl_easy INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
endfunction()

function(ftpTest)
    add_executable(ftp-test test/ftp.cpp)
    target_link_libraries(ftp-test thirdparty_curl_easy)
    if (CMAKE_SYSTEM_NAME MATCHES "Linux")
        target_link_libraries(ftp-test libunwind.a libc++abi.a libz.a dl rt)
    endif()
endfunction()

curlMain()
easyMain()
ftpTest()
