cmake_minimum_required(VERSION 3.5)
project(mfind)

set(CMAKE_C_FLAGS "-std=c11 -pthread")

set(SOURCE_FILES mfind.c mfind.c list.c mfind.h semops.h semops.c)
add_executable(mfind ${SOURCE_FILES} mfind.c)