cmake_minimum_required(VERSION 3.0)
project(main)

find_package(SystemCLanguage CONFIG REQUIRED)
set (CMAKE_CXX_STANDARD ${SystemC_CXX_STANDARD})

add_executable(main main.cpp)
target_link_libraries(main SystemC::systemc)