cmake_minimum_required(VERSION 3.0)
project(Cypress)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -g -O3 -fcolor-diagnostics -Wno-missing-braces -stdlib=libc++")

include_directories(Include/Cypress)
