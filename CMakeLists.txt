cmake_minimum_required(VERSION 3.5)
project(mfind)

set(CMAKE_C_FLAGS "-std=c11")

set(SOURCE_FILES mfind.c mfind.c list.c)
add_executable(main ${SOURCE_FILES} mfind.c)