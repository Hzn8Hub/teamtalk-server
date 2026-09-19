cmake_minimum_required(VERSION 3.10)
project(daeml)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCE_LIST
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

add_executable(daeml ${SOURCE_LIST})

target_include_directories(daeml PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_options(daeml PRIVATE -g -W -Wall)
target_compile_definitions(daeml PRIVATE
    _REENTRANT
    _FILE_OFFSET_BITS=64
    AC_HAS_INFO
    AC_HAS_WARNING
    AC_HAS_ERROR
    AC_HAS_CRITICAL
    AC_HAS_DEBUG
    TIXML_USE_STL
    LINUX_DAEMON
)

