cmake_minimum_required(VERSION 3.24)

project(
    wavlab
    VERSION 0.1.0
    DESCRIPTION "PCM WAV processing library and command-line utility"
    LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(WavLabProjectOptions)
include(FetchContent)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

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
    src/lib/FilterEffects.cpp
    src/lib/FX.cpp
    src/lib/Generators.cpp
    src/lib/LFO.cpp
    src/lib/ModulationEffects.cpp
    src/lib/Tools.cpp
    src/lib/WavFile.cpp
)

set(WAVLAB_CLI_CORE_SOURCES
    src/cli/Cli.cpp
    src/cli/EffectsCli.cpp
    src/cli/EqCli.cpp
    src/cli/GeneratorCli.cpp
    src/cli/StringHelpers.cpp
    src/cli/ToolsCli.cpp
)

find_package(CLI11 2.7.2 QUIET CONFIG)
if(NOT CLI11_FOUND)
    set(CLI11_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(CLI11_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(CLI11_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(CLI11_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        CLI11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG cbd58a3696887b34c70949aef21a71735a0c2ad5
        GIT_SHALLOW OFF
    )
    FetchContent_MakeAvailable(CLI11)
endif()

add_library(wavlab_lib ${WAVLAB_LIBRARY_SOURCES} ${WAVLAB_PUBLIC_HEADERS})
add_library(wavlab::wavlab ALIAS wavlab_lib)

set_target_properties(
    wavlab_lib
    PROPERTIES
        COMPILE_PDB_NAME wavlab_lib
        CXX_EXTENSIONS OFF
        EXPORT_NAME wavlab
        OUTPUT_NAME wavlab
        PDB_NAME wavlab_lib
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        WINDOWS_EXPORT_ALL_SYMBOLS ON
)

target_compile_features(wavlab_lib PUBLIC cxx_std_20)
target_include_directories(
    wavlab_lib
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/lib"
)
wavlab_apply_project_options(wavlab_lib)

add_library(wavlab_cli_core STATIC ${WAVLAB_CLI_CORE_SOURCES})
set_target_properties(wavlab_cli_core PROPERTIES CXX_EXTENSIONS OFF)
target_compile_features(wavlab_cli_core PRIVATE cxx_std_20)
target_include_directories(wavlab_cli_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/cli")
target_link_libraries(wavlab_cli_core PUBLIC wavlab::wavlab CLI11::CLI11)
target_compile_definitions(wavlab_cli_core PRIVATE WAVLAB_VERSION="${PROJECT_VERSION}")
wavlab_apply_project_options(wavlab_cli_core)

add_executable(wavlab_cli src/cli/main.cpp)
set_target_properties(
    wavlab_cli
    PROPERTIES
        COMPILE_PDB_NAME wavlab_cli
        CXX_EXTENSIONS OFF
        OUTPUT_NAME wavlab
        PDB_NAME wavlab_cli
)
target_compile_features(wavlab_cli PRIVATE cxx_std_20)
target_link_libraries(wavlab_cli PRIVATE wavlab_cli_core)
wavlab_apply_project_options(wavlab_cli)

install(
    TARGETS wavlab_lib
    EXPORT wavlabTargets
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
install(TARGETS wavlab_cli RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(DIRECTORY include/wavlab DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES LICENSE README.md CHANGELOG.md DESTINATION "${CMAKE_INSTALL_DOCDIR}")

set(WAVLAB_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/wavlab")
configure_package_config_file(
    cmake/wavlabConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/wavlabConfig.cmake"
    INSTALL_DESTINATION "${WAVLAB_INSTALL_CMAKEDIR}"
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/wavlabConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMinorVersion
)
install(
    EXPORT wavlabTargets
    FILE wavlabTargets.cmake
    NAMESPACE wavlab::
    DESTINATION "${WAVLAB_INSTALL_CMAKEDIR}"
)
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/wavlabConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/wavlabConfigVersion.cmake"
    DESTINATION "${WAVLAB_INSTALL_CMAKEDIR}"
)

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
    include/wavlab/Delay.h
    include/wavlab/EQ.h
    include/wavlab/FX.h
    include/wavlab/Generators.h
    include/wavlab/LFO.h
    include/wavlab/Tools.h
    include/wavlab/WavFile.h
    src/cli/Cli.cpp
    src/cli/Cli.h
    src/cli/CliCommon.h
    src/cli/EffectsCli.cpp
    src/cli/EqCli.cpp
    src/cli/GeneratorCli.cpp
    src/cli/StringHelpers.cpp
    src/cli/StringHelpers.h
    src/cli/ToolsCli.cpp
    src/cli/main.cpp
    src/lib/BinaryHelpers.cpp
    src/lib/BinaryHelpers.h
    src/lib/ConversionHelpers.cpp
    src/lib/ConversionHelpers.h
    src/lib/Delay.cpp
    src/lib/DspConversions.h
    src/lib/EQ.cpp
    src/lib/FilterEffects.cpp
    src/lib/FX.cpp
    src/lib/Generators.cpp
    src/lib/LFO.cpp
    src/lib/ModulationEffects.cpp
    src/lib/Tools.cpp
    src/lib/WavFile.cpp
    tests/binary_conversion_tests.cpp
    tests/cli_parser_tests.cpp
    tests/delay_lfo_tests.cpp
    tests/editing_tools_tests.cpp
    tests/effects_tests.cpp
    tests/eq_tests.cpp
    tests/filter_effects_tests.cpp
    tests/generator_tests.cpp
    tests/library_api_tests.cpp
    tests/modulation_effects_tests.cpp
    tests/package_consumer/main.cpp
    tests/smoke/library_smoke.cpp
)
