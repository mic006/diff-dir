# Project
cmake_minimum_required(VERSION 3.0.0)
project(Diff-Dir VERSION 0)

# C++ compiler settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
string(APPEND CMAKE_CXX_FLAGS " -Wall -Wextra -Werror -pipe")
string(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize=address -fsanitize=undefined")
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

# yaml-cpp
find_package(yaml-cpp)

# https://beesbuzz.biz/code/4399-Embedding-binary-resources-with-CMake-and-C-11
# Compilation step for static resources
function(add_resources out_var)
  set(result)
  foreach(in_f ${ARGN})
    file(RELATIVE_PATH src_f ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${in_f})
    set(out_f "${PROJECT_BINARY_DIR}/${in_f}.o")
    add_custom_command(OUTPUT ${out_f}
      COMMAND ld -r -b binary -o ${out_f} ${src_f}
      DEPENDS ${in_f}
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      COMMENT "Adding resource ${out_f}"
      VERBATIM
    )
    list(APPEND result ${out_f})
  endforeach()
  set(${out_var} "${result}" PARENT_SCOPE)
endfunction()

add_resources(default_config "diff-dir.conf.yaml")

# main executable
add_executable(diff-dir
    src/context.cpp
    src/diff_dir.cpp
    src/dispatcher.cpp
    src/dispatcher_mono.cpp
    src/dispatcher_multi.cpp
    src/file_comp.cpp
    src/ignore.cpp
    src/main.cpp
    src/path.cpp
    src/report.cpp
    src/report_compact.cpp
    src/report_interactive.cpp
    src/term_app.cpp
    src/term_app_settings.cpp
    src/text_diff.cpp
    src/yaml_util.cpp
    termui/termui.cpp
    termui/termui_internal.cpp
    ${default_config}
)
target_compile_definitions(diff-dir PUBLIC
    VERSION="${APP_VERSION}"
)
target_include_directories(diff-dir PUBLIC
    cxxopts
    dtl
    termui
)
target_link_libraries(diff-dir PUBLIC
    pthread
    yaml-cpp
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
