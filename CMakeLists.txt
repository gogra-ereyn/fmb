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

add_executable(fmb src/fmb.c)
function(create_bin_link tgt)
    add_custom_command(TARGET ${tgt} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            build/$<TARGET_FILE_NAME:${tgt}>
            ${CMAKE_SOURCE_DIR}/${tgt}
        COMMENT "Creating symlink to ${tgt} in project root"
    )
endfunction()
create_bin_link(fmb)

enable_testing()

create_bin_link(fmb)
add_test(
    NAME run_tests COMMAND bash ${CMAKE_SOURCE_DIR}/tests/test.sh
)

