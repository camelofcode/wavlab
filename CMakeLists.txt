cmake_minimum_required(VERSION 3.24)

project(
    wavlab
    VERSION 0.1.0
    DESCRIPTION "PCM WAV processing library and command-line utility"
    LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(WavLabProjectOptions)

option(BUILD_SHARED_LIBS "Build wavlab as a shared library" OFF)

set(WAVLAB_PUBLIC_HEADERS
    WavLab/BinaryHelpers.h
    WavLab/Commands.h
    WavLab/Constants.h
    WavLab/ConversionHelpers.h
    WavLab/Delay.h
    WavLab/EQ.h
    WavLab/FX.h
    WavLab/Generators.h
    WavLab/LFO.h
    WavLab/StringHelpers.h
    WavLab/Tools.h
    WavLab/TryParse.h
    WavLab/WavFile.h
)

set(WAVLAB_LIBRARY_SOURCES
    WavLab/BinaryHelpers.cpp
    WavLab/Commands.cpp
    WavLab/ConversionHelpers.cpp
    WavLab/Delay.cpp
    WavLab/EQ.cpp
    WavLab/FX.cpp
    WavLab/Generators.cpp
    WavLab/LFO.cpp
    WavLab/StringHelpers.cpp
    WavLab/Tools.cpp
    WavLab/WavFile.cpp
)

add_library(wavlab_lib ${WAVLAB_LIBRARY_SOURCES} ${WAVLAB_PUBLIC_HEADERS})
add_library(wavlab::wavlab ALIAS wavlab_lib)

set_target_properties(
    wavlab_lib
    PROPERTIES
        COMPILE_PDB_NAME wavlab_lib
        CXX_EXTENSIONS OFF
        OUTPUT_NAME wavlab
        PDB_NAME wavlab_lib
        WINDOWS_EXPORT_ALL_SYMBOLS ON
)

target_compile_features(wavlab_lib PUBLIC cxx_std_20)
target_include_directories(
    wavlab_lib
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/WavLab>"
        "$<INSTALL_INTERFACE:include>"
)
wavlab_apply_project_options(wavlab_lib)

add_executable(wavlab_cli WavLab/WavLab.cpp)
set_target_properties(
    wavlab_cli
    PROPERTIES
        COMPILE_PDB_NAME wavlab_cli
        CXX_EXTENSIONS OFF
        OUTPUT_NAME wavlab
        PDB_NAME wavlab_cli
)
target_compile_features(wavlab_cli PRIVATE cxx_std_20)
target_link_libraries(wavlab_cli PRIVATE wavlab::wavlab)
wavlab_apply_project_options(wavlab_cli)

include(CTest)
if(BUILD_TESTING AND PROJECT_IS_TOP_LEVEL)
    add_subdirectory(tests)
endif()

wavlab_add_developer_targets(
    tests/smoke/library_smoke.cpp
)
