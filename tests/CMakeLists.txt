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
wavlab_apply_project_options(wavlab_api_tests)
add_test(NAME wavlab.library.api COMMAND wavlab_api_tests)

add_test(NAME wavlab.cli.help COMMAND wavlab_cli --help)
set_tests_properties(
    wavlab.cli.help
    PROPERTIES
        PASS_REGULAR_EXPRESSION "wavlab commands"
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
