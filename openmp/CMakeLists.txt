cmake_minimum_required(VERSION 3.16)

project(openmplab VERSION 0.1.0 LANGUAGES C)

set(target main)

set(src_dir ${CMAKE_SOURCE_DIR}/src)

file(GLOB src_files ${src_dir}/*.c ${src_dir}/*.cpp)

add_executable(${target} ${src_files})

# target_include_directories(${target} PRIVATE ${ompi_dir})
target_compile_options(${target} PRIVATE "-fopenmp")


