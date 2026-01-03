cmake_minimum_required(VERSION 3.15)
project(fmb)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_STANDARD 99)
set(FAST_OPTS "-O3 -march=native -pipe")
set(CMAKE_C_FLAGS_RELEASE          "${FAST_OPTS} -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO   "${FAST_OPTS} -g")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

enable_testing()
add_executable(fmb ${CMAKE_CURRENT_SOURCE_DIR}/fmb.c)
add_test(
    NAME run_tests COMMAND bash ${CMAKE_SOURCE_DIR}/tests/test.sh
)

