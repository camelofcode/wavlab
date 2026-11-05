cmake_minimum_required(VERSION 3.24)

project(wavlab_package_consumer LANGUAGES CXX)

find_package(wavlab CONFIG REQUIRED)

add_executable(wavlab_package_consumer main.cpp)
set_target_properties(wavlab_package_consumer PROPERTIES CXX_EXTENSIONS OFF)
target_compile_features(wavlab_package_consumer PRIVATE cxx_std_20)
target_link_libraries(wavlab_package_consumer PRIVATE wavlab::wavlab)

enable_testing()
add_test(NAME wavlab.package.consumer.runs COMMAND wavlab_package_consumer)
