cmake_minimum_required(VERSION 3.15)  # CMake version check
project(wavlab)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 14)            # Enable c++14 standard

set(CMAKE_EXE_LINKER_FLAGS "-static") #include libraries

# Ensure warnings logged
if ( CMAKE_COMPILER_IS_GNUCC )
    #Ignore unused-parameter as it warns about functions passed to registerCommand which must match std::function<int(std::string command, int argc, char* argv[])> action
    #Ignore reorder as initialisation order doesn't matter too much right now
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-reorder")
endif()
if ( MSVC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /W4")
    string(REGEX REPLACE "/W[3|4]" "/w" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()


add_executable(wavlab WavLab/WavLab.cpp WavLab/WavFile.cpp WavLab/Tools.cpp WavLab/StringHelpers.cpp WavLab/Generators.cpp WavLab/FX.cpp WavLab/EQ.cpp WavLab/ConversionHelpers.cpp WavLab/Commands.cpp WavLab/BinaryHelpers.cpp)