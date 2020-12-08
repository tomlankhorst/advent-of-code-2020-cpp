cmake_minimum_required(VERSION 3.17)
project(advent_of_code)

set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(day1-twentytwenty day1/twentytwenty.cpp)
add_executable(day2-password-philosophy day2/password-philosophy.cpp)
add_executable(day3-toboggan-trajectory day3/toboggan-trajectory.cpp)
add_executable(day4-passport-processing day4/passport-processing.cpp)
add_executable(day5-binary-boarding day5/binary-boarding.cpp)
add_executable(day6-custom-customs day6/custom-customs.cpp)
add_executable(day7-handy-haversacks day7/handy-haversacks.cpp)
add_executable(day8-handheld-halting day8/handheld-halting.cpp)
