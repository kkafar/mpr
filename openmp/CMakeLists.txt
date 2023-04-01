cmake_minimum_required(VERSION 3.16)

project(playground VERSION 0.1.0 LANGUAGES C)

set(target main)


set(src_dir ${CMAKE_SOURCE_DIR}/src)
set(ompi_dir "/home/kkafara/software/dependencies/include")

file(GLOB src_files ${src_dir}/*.c)

add_executable(${target} ${src_files})

target_include_directories(${target} PRIVATE ${ompi_dir})


