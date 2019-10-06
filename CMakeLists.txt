cmake_minimum_required(VERSION 3.10)
project(config_generator)

set(CMAKE_CXX_STANDARD 17)

add_executable(config_generator main.cpp src/generator_parameters.cpp src/generator_parameters.h src/config_generator.cpp src/config_generator.h src/string_utils.h src/parsing_utils.h)
