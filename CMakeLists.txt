cmake_minimum_required(VERSION 3.20)

project(ML-Reader LANGUAGES CXX)

list(APPEND
    CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

include(GNUInstallDirs)
include(PreventInSourceBuilds)
include(CompilerWarnings)
include(GitVersion)

add_library(project_config INTERFACE)
set_compiler_warnings(project_config INTERFACE)
target_compile_features(project_config INTERFACE cxx_std_20)

include(LoadQt)
include(LoadQtcsv)
include(LoadQtKeychain)

add_subdirectory(src/main)
add_subdirectory(src/mlclient)
