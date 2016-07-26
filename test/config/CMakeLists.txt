## jflags package configuration tests

cmake_minimum_required (VERSION 2.8.12 FATAL_ERROR)

project (jflags_${TEST_NAME})

find_package (jflags REQUIRED)

add_executable (foo main.cc)
target_link_libraries (foo jflags)
