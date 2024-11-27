# Copyright 2021 Varjo Technologies Oy. All rights reserved.

# Project name
set(_lib_name "VarjoTrackerLib")
project(${_lib_name})

# Runtime output directories
set(_build_output_dir ${CMAKE_BINARY_DIR}/bin)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${_build_output_dir})
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

# Public common sources
set(_src_common_dir ${CMAKE_CURRENT_SOURCE_DIR}/sdk/src)
set(_sources_common
    ${_src_common_dir}/Session.cpp
    ${_src_common_dir}/Session.hpp
)

# Library sources
set(_src_dir ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(_sources_lib
    ${_src_dir}/Application.cpp
    ${_src_dir}/Application.hpp
    ${_src_dir}/GazeTracking.cpp
    ${_src_dir}/GazeTracking.hpp
    ${_src_dir}/lib.cpp
)

# Library target
set(_target ${_lib_name})
add_library(${_target} SHARED 
    ${_sources_lib}
    ${_sources_common}
)

# Visual studio source groups
source_group("Library" FILES ${_sources_lib})
source_group("Common" FILES ${_sources_common})

# Include directories
target_include_directories(${_target} PRIVATE ${_src_common_dir})
target_include_directories(${_target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/sdk/include)
target_link_directories(${_target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/sdk/lib)

# Linked libraries
target_link_libraries(${_target}
    PRIVATE VarjoLib
)

# VS properties
set_target_properties(${_target} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set_property(TARGET ${_target} PROPERTY FOLDER "Source")
set_property(TARGET ${_target} PROPERTY CXX_STANDARD 17)

# Preprocerssor definitions
target_compile_definitions(${_target} PUBLIC -D_UNICODE -DUNICODE)

# Copy Varjo's 'DLLs to the output folder
add_custom_command(TARGET ${_target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/sdk/bin/VarjoLib.dll
        $<TARGET_FILE_DIR:${_target}>)
