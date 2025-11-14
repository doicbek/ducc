# CMakeLists.txt for DUCC0 MATLAB MEX Interface
cmake_minimum_required(VERSION 3.15)

project(ducc0_matlab_mex)

# Find MATLAB
find_program(MATLAB_EXECUTABLE matlab PATHS
    "C:/Program Files/MATLAB/*/bin"
    "/usr/local/MATLAB/*/bin"
    "$ENV{MATLAB_ROOT}/bin"
)

if(NOT MATLAB_EXECUTABLE)
    message(FATAL_ERROR "MATLAB not found. Please set MATLAB_ROOT or add MATLAB to PATH")
endif()

# Get MATLAB root directory
get_filename_component(MATLAB_ROOT ${MATLAB_EXECUTABLE} DIRECTORY)
get_filename_component(MATLAB_ROOT ${MATLAB_ROOT} DIRECTORY)

# Find MEX compiler
find_program(MEX_EXECUTABLE mex PATHS
    "${MATLAB_ROOT}/bin"
    NO_DEFAULT_PATH
)

if(NOT MEX_EXECUTABLE)
    message(FATAL_ERROR "MEX compiler not found")
endif()

# Get MATLAB include directory
set(MATLAB_INCLUDE_DIR "${MATLAB_ROOT}/extern/include")

# DUCC source directory (relative to this file)
set(DUCC_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../src")

# Include directories
include_directories(
    ${MATLAB_INCLUDE_DIR}
    ${DUCC_SRC_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Source files for DUCC (needed for linking)
set(DUCC_SOURCES
    ${DUCC_SRC_DIR}/ducc0/infra/threading.cc
    ${DUCC_SRC_DIR}/ducc0/infra/mav.cc
    ${DUCC_SRC_DIR}/ducc0/infra/string_utils.cc
    ${DUCC_SRC_DIR}/ducc0/math/gl_integrator.cc
    ${DUCC_SRC_DIR}/ducc0/math/gridding_kernel.cc
    ${DUCC_SRC_DIR}/ducc0/math/pointing.cc
    ${DUCC_SRC_DIR}/ducc0/math/wigner3j.cc
    ${DUCC_SRC_DIR}/ducc0/math/space_filling.cc
    ${DUCC_SRC_DIR}/ducc0/fft/fft_inst1.cc
    ${DUCC_SRC_DIR}/ducc0/fft/fft_inst2.cc
    ${DUCC_SRC_DIR}/ducc0/nufft/spreadinterp_inst1.cc
    ${DUCC_SRC_DIR}/ducc0/nufft/spreadinterp_inst2.cc
    ${DUCC_SRC_DIR}/ducc0/sht/sht.cc
    ${DUCC_SRC_DIR}/ducc0/wgridder/wgridder.cc
    ${DUCC_SRC_DIR}/ducc0/wgridder/wgridder_inst1.cc
    ${DUCC_SRC_DIR}/ducc0/wgridder/wgridder_inst2.cc
    ${DUCC_SRC_DIR}/ducc0/wgridder/wgridder_inst3.cc
    ${DUCC_SRC_DIR}/ducc0/healpix/healpix_base.cc
    ${DUCC_SRC_DIR}/ducc0/healpix/healpix_tables.cc
)

# MEX source files
set(MEX_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/ducc0_fft_mex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ducc0_fft_r2c_mex.cpp
)

# Compiler flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W3 /EHsc)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wfatal-errors -pthread)
    add_compile_options(-O3 -march=native -ffast-math)
endif()

# Custom command to build MEX files
foreach(mex_file ${MEX_SOURCES})
    get_filename_component(mex_name ${mex_file} NAME_WE)
    
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${mex_name}.${MEX_EXT}
        COMMAND ${MEX_EXECUTABLE}
            -I${MATLAB_INCLUDE_DIR}
            -I${DUCC_SRC_DIR}
            -I${CMAKE_CURRENT_SOURCE_DIR}
            -cxx
            -outdir ${CMAKE_CURRENT_BINARY_DIR}
            ${mex_file}
            ${DUCC_SOURCES}
        DEPENDS ${mex_file} ${DUCC_SOURCES}
        COMMENT "Building MEX file ${mex_name}"
    )
    
    add_custom_target(${mex_name}_mex ALL
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${mex_name}.${MEX_EXT}
    )
endforeach()

# Note: MEX_EXT will be determined at configure time
# For Windows: .mexw64 or .mexw32
# For Linux: .mexa64
# For Mac: .mexmaci64

