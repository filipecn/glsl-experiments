# avoid policy warning message
if (POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)
# main project
project(GLSL_EXPERIMENTS)
cmake_minimum_required(VERSION 3.1)
cmake_policy(VERSION 3.1)
set(CMAKE_VERBOSE_MAKEFILE ON)
# path variables
set(ROOT_PROJECT_PATH "${PROJECT_SOURCE_DIR}")
set(BUILD_ROOT ${CMAKE_CURRENT_BINARY_DIR})
##########################################
##               options                ##
##########################################
#option(BUILD_ALL "build all targets" OFF)
# cmake modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})
# check for specific machine/compiler options.
include(ConfigCompilerFlags)
##########################################
##             dependencies             ##
##########################################
include(ExternalProject)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/ext")
include(ponos)
##########################################
##               sources                ##
##########################################
set(SOURCES
        src/shader_editor.cpp)
add_executable(glsl_editor ${SOURCES})
target_compile_definitions(glsl_editor PUBLIC
        -DSHADERS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
        -DMODELS_PATH="${CMAKE_CURRENT_SOURCE_DIR}"
        -DTEXTURES_PATH="${CMAKE_CURRENT_SOURCE_DIR}"
        )
#add_dependencies(glsl_editor ponos)
target_include_directories(glsl_editor PUBLIC ${PONOS_INCLUDES})
target_link_libraries(glsl_editor  ${PONOS_LIBRARIES})

