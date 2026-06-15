cmake_minimum_required(VERSION 3.21)

project(TS_RS_App
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(tsrs_app_core STATIC
    src/core/AppBuildInfo.cpp
)

target_include_directories(tsrs_app_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(tsrs_app_core PUBLIC cxx_std_17)

enable_testing()
add_subdirectory(tests)
