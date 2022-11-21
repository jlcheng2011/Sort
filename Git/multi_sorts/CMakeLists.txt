cmake_minimum_required(VERSION 3.23)
project(multi_sorts)

include_directories(SYSTEM "BasicSort")


set(CMAKE_CXX_STANDARD 14)


add_executable(multi_sorts main.cpp BasicSort/SortFactory.cpp)
