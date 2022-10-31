cmake_minimum_required(VERSION 3.13)  # CMake version check
project(wavlab)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 14)            # Enable c++14 standard

add_executable(wavlab WavLab.cpp WavFile.cpp Tools.cpp StringHelpers.cpp Generators.cpp FX.cpp EQ.cpp ConversionHelpers.cpp Commands.cpp BinaryHelpers.cpp)