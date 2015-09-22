


cmake_minimum_required(VERSION 3.0)


set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")


project(Cypress)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wall -Wextra -g -O0 -fcolor-diagnostics -Wno-missing-braces -stdlib=libc++")

include_directories(Include)
include_directories(/usr/local/include)
include_directories(/usr/include/mpi)

find_package(Boost COMPONENTS program_options system filesystem REQUIRED)

add_subdirectory(gtest)
add_subdirectory(Source)
add_subdirectory(Test)
