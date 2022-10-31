cmake_minimum_required(VERSION 3.13)  # CMake version check
project(wavlab)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 14)            # Enable c++14 standard
set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(wavlab WavLab/WavLab.cpp WavLab/WavFile.cpp WavLab/Tools.cpp WavLab/StringHelpers.cpp WavLab/Generators.cpp WavLab/FX.cpp WavLab/EQ.cpp WavLab/ConversionHelpers.cpp WavLab/Commands.cpp WavLab/BinaryHelpers.cpp)