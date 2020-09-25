# Project
cmake_minimum_required(VERSION 3.0.0)
project(Diff-Dir VERSION 0)

# C++ compiler settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
string(APPEND CMAKE_CXX_FLAGS " -Wall -Wextra -Werror -pipe")
string(APPEND CMAKE_CXX_FLAGS_DEBUG " -O1")
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -flto -s")

# get APP_VERSION from git describe
find_package(Git QUIET REQUIRED)
execute_process(
    COMMAND "${GIT_EXECUTABLE}" describe --tags --always --dirty
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    RESULT_VARIABLE res
    OUTPUT_VARIABLE APP_VERSION
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
set_property(GLOBAL APPEND
    PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/.git/index"
)

# main executable
add_executable(diff-dir
    src/diff_dir.cpp
    src/dispatcher.cpp
    src/dispatcher_mono.cpp
    src/dispatcher_multi.cpp
    src/file_comp.cpp
    src/ignore.cpp
    src/main.cpp
    src/path.cpp
    src/report.cpp
    src/report_console.cpp
)
target_compile_definitions(diff-dir PUBLIC
    VERSION="${APP_VERSION}"
)
target_include_directories(diff-dir PUBLIC
    cxxopts
)
target_link_libraries(diff-dir PUBLIC
    pthread
)

# google test
find_package(GTest)
add_executable(test-diff-dir
    src/file_comp.cpp
    src/ignore.cpp
    src/test/test_file_comp.cpp
    src/test/test_ignore.cpp
)
target_link_libraries(test-diff-dir
    gtest
    gtest_main
    pthread
)
add_test(NAME test-diff-dir COMMAND test-diff-dir)
