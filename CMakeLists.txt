cmake_minimum_required(VERSION 3.10)
project(config-generator)

set(CMAKE_CXX_STANDARD 17)

add_executable(config-generator main.cpp src/generator_parameters.cpp src/generator_parameters.h src/config_generator.cpp src/config_generator.h src/string_utils.h src/parsing_utils.h)

install (TARGETS config-generator DESTINATION bin)