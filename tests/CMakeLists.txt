find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_executable(wavlab_tests smoke/library_smoke.cpp)
set_target_properties(wavlab_tests PROPERTIES CXX_EXTENSIONS OFF)
target_compile_features(wavlab_tests PRIVATE cxx_std_20)
target_link_libraries(wavlab_tests PRIVATE wavlab::wavlab)
wavlab_apply_project_options(wavlab_tests)

add_test(NAME wavlab.library.smoke COMMAND wavlab_tests)

add_executable(wavlab_api_tests library_api_tests.cpp)
set_target_properties(wavlab_api_tests PROPERTIES CXX_EXTENSIONS OFF)
target_compile_features(wavlab_api_tests PRIVATE cxx_std_20)
target_link_libraries(wavlab_api_tests PRIVATE wavlab::wavlab)
target_compile_definitions(
    wavlab_api_tests
    PRIVATE
        WAVLAB_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)
wavlab_apply_project_options(wavlab_api_tests)
add_test(NAME wavlab.library.api COMMAND wavlab_api_tests)

add_executable(wavlab_binary_conversion_tests binary_conversion_tests.cpp)
set_target_properties(wavlab_binary_conversion_tests PROPERTIES CXX_EXTENSIONS OFF)
target_compile_features(wavlab_binary_conversion_tests PRIVATE cxx_std_20)
target_include_directories(
    wavlab_binary_conversion_tests
    PRIVATE
        "${PROJECT_SOURCE_DIR}/src/lib"
)
target_link_libraries(wavlab_binary_conversion_tests PRIVATE wavlab::wavlab)
wavlab_apply_project_options(wavlab_binary_conversion_tests)
add_test(NAME wavlab.library.binary-conversion COMMAND wavlab_binary_conversion_tests)

add_executable(wavlab_editing_tools_tests editing_tools_tests.cpp)
set_target_properties(wavlab_editing_tools_tests PROPERTIES CXX_EXTENSIONS OFF)
target_compile_features(wavlab_editing_tools_tests PRIVATE cxx_std_20)
target_link_libraries(wavlab_editing_tools_tests PRIVATE wavlab::wavlab)
wavlab_apply_project_options(wavlab_editing_tools_tests)
add_test(NAME wavlab.library.editing-tools COMMAND wavlab_editing_tools_tests)

add_executable(wavlab_generator_tests generator_tests.cpp)
set_target_properties(wavlab_generator_tests PROPERTIES CXX_EXTENSIONS OFF)
target_compile_features(wavlab_generator_tests PRIVATE cxx_std_20)
target_link_libraries(wavlab_generator_tests PRIVATE wavlab::wavlab)
wavlab_apply_project_options(wavlab_generator_tests)
add_test(NAME wavlab.library.generators COMMAND wavlab_generator_tests)

add_executable(wavlab_eq_tests eq_tests.cpp)
set_target_properties(wavlab_eq_tests PROPERTIES CXX_EXTENSIONS OFF)
target_compile_features(wavlab_eq_tests PRIVATE cxx_std_20)
target_link_libraries(wavlab_eq_tests PRIVATE wavlab::wavlab)
wavlab_apply_project_options(wavlab_eq_tests)
add_test(NAME wavlab.library.eq COMMAND wavlab_eq_tests)

add_executable(wavlab_delay_lfo_tests delay_lfo_tests.cpp)
set_target_properties(wavlab_delay_lfo_tests PROPERTIES CXX_EXTENSIONS OFF)
target_compile_features(wavlab_delay_lfo_tests PRIVATE cxx_std_20)
target_link_libraries(wavlab_delay_lfo_tests PRIVATE wavlab::wavlab)
wavlab_apply_project_options(wavlab_delay_lfo_tests)
add_test(NAME wavlab.library.delay-lfo COMMAND wavlab_delay_lfo_tests)

add_executable(wavlab_effects_tests effects_tests.cpp)
set_target_properties(wavlab_effects_tests PROPERTIES CXX_EXTENSIONS OFF)
target_compile_features(wavlab_effects_tests PRIVATE cxx_std_20)
target_link_libraries(wavlab_effects_tests PRIVATE wavlab::wavlab)
wavlab_apply_project_options(wavlab_effects_tests)
add_test(NAME wavlab.library.effects COMMAND wavlab_effects_tests)

add_test(NAME wavlab.cli.help COMMAND wavlab_cli --help)
set_tests_properties(
    wavlab.cli.help
    PROPERTIES
        PASS_REGULAR_EXPRESSION "wavlab commands"
)
add_test(
    NAME wavlab.cli.independent-reader
    COMMAND
        "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/verify_cli_wav.py"
        "$<TARGET_FILE:wavlab_cli>"
)
add_test(
    NAME wavlab.cli.editing-tools
    COMMAND
        "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/editing_cli_tests.py"
        "$<TARGET_FILE:wavlab_cli>"
)
add_test(
    NAME wavlab.fixtures.current
    COMMAND
        "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/generate_fixtures.py"
        --check
)

set(WAVLAB_HEADER_SMOKE_SOURCES)
foreach(header IN LISTS WAVLAB_PUBLIC_HEADERS)
    get_filename_component(header_name "${header}" NAME_WE)
    string(MAKE_C_IDENTIFIER "${header_name}" header_identifier)
    set(generated_source "${CMAKE_CURRENT_BINARY_DIR}/header_${header_identifier}.cpp")
    file(
        GENERATE
        OUTPUT "${generated_source}"
        CONTENT "#include <wavlab/${header_name}.h>\n"
    )
    list(APPEND WAVLAB_HEADER_SMOKE_SOURCES "${generated_source}")
endforeach()

add_library(wavlab_header_smoke OBJECT ${WAVLAB_HEADER_SMOKE_SOURCES})
set_target_properties(wavlab_header_smoke PROPERTIES CXX_EXTENSIONS OFF)
target_compile_features(wavlab_header_smoke PRIVATE cxx_std_20)
target_link_libraries(wavlab_header_smoke PRIVATE wavlab::wavlab)
wavlab_apply_project_options(wavlab_header_smoke)
