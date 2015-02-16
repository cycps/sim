cmake_minimum_required(VERSION 3.0)
project(Cypress)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wall -Wextra -g -O3 -fcolor-diagnostics -Wno-missing-braces -stdlib=libc++")

include_directories(Include)
include_directories(/usr/local/include)

find_package(Boost COMPONENTS program_options system filesystem REQUIRED)

add_subdirectory(Source)
add_subdirectory(gtest)
add_subdirectory(Test)
