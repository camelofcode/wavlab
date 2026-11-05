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
    include/wavlab/Delay.h
    include/wavlab/EQ.h
    include/wavlab/FX.h
    include/wavlab/Generators.h
    include/wavlab/LFO.h
    include/wavlab/Tools.h
    include/wavlab/WavFile.h
)

set(WAVLAB_LIBRARY_SOURCES
    src/lib/BinaryHelpers.cpp
    src/lib/ConversionHelpers.cpp
    src/lib/Delay.cpp
    src/lib/EQ.cpp
    src/lib/FX.cpp
    src/lib/Generators.cpp
    src/lib/LFO.cpp
    src/lib/Tools.cpp
    src/lib/WavFile.cpp
)

set(WAVLAB_CLI_SOURCES
    src/cli/Commands.cpp
    src/cli/EQCommands.cpp
    src/cli/FXCommands.cpp
    src/cli/GeneratorCommands.cpp
    src/cli/StringHelpers.cpp
    src/cli/ToolsCommands.cpp
    src/cli/main.cpp
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
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:include>"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/lib"
)
wavlab_apply_project_options(wavlab_lib)

add_executable(wavlab_cli ${WAVLAB_CLI_SOURCES})
set_target_properties(
    wavlab_cli
    PROPERTIES
        COMPILE_PDB_NAME wavlab_cli
        CXX_EXTENSIONS OFF
        OUTPUT_NAME wavlab
        PDB_NAME wavlab_cli
)
target_compile_features(wavlab_cli PRIVATE cxx_std_20)
target_include_directories(wavlab_cli PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/cli")
target_link_libraries(wavlab_cli PRIVATE wavlab::wavlab)
wavlab_apply_project_options(wavlab_cli)

add_executable(wavlab_normalise_example examples/normalise.cpp)
set_target_properties(wavlab_normalise_example PROPERTIES CXX_EXTENSIONS OFF)
target_compile_features(wavlab_normalise_example PRIVATE cxx_std_20)
target_link_libraries(wavlab_normalise_example PRIVATE wavlab::wavlab)
wavlab_apply_project_options(wavlab_normalise_example)

include(CTest)
if(BUILD_TESTING AND PROJECT_IS_TOP_LEVEL)
    add_subdirectory(tests)
endif()

wavlab_add_developer_targets(
    examples/normalise.cpp
    include/wavlab/FX.h
    include/wavlab/Tools.h
    include/wavlab/WavFile.h
    src/cli/Commands.cpp
    src/cli/Commands.h
    src/cli/StringHelpers.cpp
    src/cli/StringHelpers.h
    src/cli/ToolsCommands.cpp
    src/lib/FX.cpp
    src/lib/BinaryHelpers.cpp
    src/lib/BinaryHelpers.h
    src/lib/ConversionHelpers.cpp
    src/lib/ConversionHelpers.h
    src/lib/Tools.cpp
    src/lib/WavFile.cpp
    tests/binary_conversion_tests.cpp
    tests/editing_tools_tests.cpp
    tests/library_api_tests.cpp
    tests/smoke/library_smoke.cpp
)
