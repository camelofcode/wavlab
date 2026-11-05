include_guard(GLOBAL)

option(WAVLAB_WARNINGS_AS_ERRORS "Treat WavLab compiler warnings as errors" OFF)
option(WAVLAB_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(WAVLAB_ENABLE_CLANG_TIDY "Run clang-tidy while compiling WavLab targets" OFF)

add_library(wavlab_warnings INTERFACE)
add_library(wavlab_sanitizers INTERFACE)

if(MSVC)
    target_compile_options(wavlab_warnings INTERFACE /W4 /permissive-)
    if(WAVLAB_WARNINGS_AS_ERRORS)
        target_compile_options(wavlab_warnings INTERFACE /WX)
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(wavlab_warnings INTERFACE -Wall -Wextra -Wpedantic)
    if(WAVLAB_WARNINGS_AS_ERRORS)
        target_compile_options(wavlab_warnings INTERFACE -Werror)
    endif()
endif()

if(WAVLAB_ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(
            wavlab_sanitizers
            INTERFACE
                -fsanitize=address,undefined
                -fno-omit-frame-pointer
        )
        target_link_options(wavlab_sanitizers INTERFACE -fsanitize=address,undefined)
    else()
        message(FATAL_ERROR "WAVLAB_ENABLE_SANITIZERS requires GCC or Clang")
    endif()
endif()

if(WAVLAB_ENABLE_CLANG_TIDY)
    find_program(WAVLAB_CLANG_TIDY_EXECUTABLE NAMES clang-tidy REQUIRED)
endif()

function(wavlab_apply_project_options target)
    target_link_libraries(
        ${target}
        PRIVATE
            "$<BUILD_INTERFACE:wavlab_warnings>"
            "$<BUILD_INTERFACE:wavlab_sanitizers>"
    )
    if(WAVLAB_ENABLE_CLANG_TIDY)
        set_target_properties(
            ${target}
            PROPERTIES
                CXX_CLANG_TIDY "${WAVLAB_CLANG_TIDY_EXECUTABLE}"
        )
    endif()
endfunction()

function(wavlab_add_developer_targets)
    find_program(WAVLAB_CLANG_FORMAT_EXECUTABLE NAMES clang-format)
    if(NOT WAVLAB_CLANG_FORMAT_EXECUTABLE)
        return()
    endif()

    set(files)
    foreach(file IN LISTS ARGN)
        list(APPEND files "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
    endforeach()

    add_custom_target(
        format
        COMMAND "${WAVLAB_CLANG_FORMAT_EXECUTABLE}" -i ${files}
        COMMENT "Formatting WavLab C++ sources"
        VERBATIM
    )
    add_custom_target(
        format-check
        COMMAND "${WAVLAB_CLANG_FORMAT_EXECUTABLE}" --dry-run --Werror ${files}
        COMMENT "Checking WavLab C++ source formatting"
        VERBATIM
    )
endfunction()
