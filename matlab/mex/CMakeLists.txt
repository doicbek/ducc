# CMakeLists.txt for DUCC0 MATLAB MEX Interface
cmake_minimum_required(VERSION 3.15)

project(ducc0_matlab_mex)

# Find MATLAB
find_program(MATLAB_EXECUTABLE matlab PATHS
    "C:/Program Files/MATLAB/*/bin"
    "C:/Program Files (x86)/MATLAB/*/bin"
    "/usr/local/MATLAB/*/bin"
    "/Applications/MATLAB_*.app/bin"
    "$ENV{MATLAB_ROOT}/bin"
    NO_DEFAULT_PATH
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

# Determine MEX extension based on platform
if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(MEX_EXT "mexw64")
    else()
        set(MEX_EXT "mexw32")
    endif()
elseif(APPLE)
    set(MEX_EXT "mexmaci64")
else()
    set(MEX_EXT "mexa64")
endif()

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
    ${CMAKE_CURRENT_SOURCE_DIR}/ducc0_fft_c2c_mex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ducc0_fft_good_size_mex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ducc0_fft_r2c_mex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ducc0_fft_c2r_mex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ducc0_fft_r2r_fftpack_mex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ducc0_fft_dct_mex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ducc0_fft_dst_mex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ducc0_fft_hartley_mex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ducc0_sht_synthesis_2d_mex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ducc0_sht_analysis_2d_mex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ducc0_sht_get_gridweights_mex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ducc0_sht_rotate_alm_mex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ducc0_sht_adjoint_synthesis_2d_mex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ducc0_sht_adjoint_analysis_2d_mex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ducc0_nufft_nu2u_mex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ducc0_nufft_u2nu_mex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ducc0_healpix_ang2pix_mex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ducc0_healpix_pix2ang_mex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ducc0_misc_vdot_mex.cpp
)

# Compiler flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# On Unix, MEX reads compiler flags from mexopts.sh
# We need to directly modify the mexopts.sh file that MEX uses
# First, find it, then modify it in place to add -std=c++17
if(UNIX)
    set(WRAPPER_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/mex_wrapper.sh")
    # Strategy: Find the actual mexopts.sh file MEX uses, modify it directly
    # This is more aggressive but should work
    file(WRITE ${WRAPPER_SCRIPT}
"#!/bin/bash
# Find and modify the mexopts.sh file that MEX actually uses
# MEX stores this in a user-specific location or MATLAB installation

# Try to find mexopts.sh
MEXOPTS_FILE=\"\"
# Check user's MATLAB preferences directory first (most common)
MATLAB_PREFS=\"$HOME/.matlab\"
if [ -d \"$MATLAB_PREFS\" ]; then
    # Look for mexopts.sh in version-specific or general location
    for ver_dir in \"$MATLAB_PREFS\"/*/mexopts.sh \"$MATLAB_PREFS\"/mexopts.sh; do
        if [ -f \"$ver_dir\" ]; then
            MEXOPTS_FILE=\"$ver_dir\"
            break
        fi
    done
fi

# If not found, try MATLAB bin directory
if [ -z \"$MEXOPTS_FILE\" ] && [ -f \"${MATLAB_ROOT}/bin/mexopts.sh\" ]; then
    MEXOPTS_FILE=\"${MATLAB_ROOT}/bin/mexopts.sh\"
fi

# If still not found, try to get it from MEX
if [ -z \"$MEXOPTS_FILE\" ]; then
    # Run mex -setup to see where it points
    MEX_OUTPUT=\"$(${MEX_EXECUTABLE} -setup 2>&1 || true)\"
    # Try to extract path (this is a fallback)
fi

# If we found mexopts.sh, modify it
if [ -n \"$MEXOPTS_FILE\" ] && [ -f \"$MEXOPTS_FILE\" ]; then
    # Backup original
    cp \"$MEXOPTS_FILE\" \"$MEXOPTS_FILE.bak\" 2>/dev/null || true
    
    # Modify CXXFLAGS to include -std=c++17
    # Try multiple sed patterns to handle different formats
    if ! grep -q '\"-std=c++17\"' \"$MEXOPTS_FILE\" && ! grep -q \"'-std=c++17'\" \"$MEXOPTS_FILE\"; then
        # Pattern 1: CXXFLAGS=\"...\"
        sed -i.tmp 's/\\(CXXFLAGS=\"[^\"]*\\)\"/\\1 -std=c++17\"/' \"$MEXOPTS_FILE\" 2>/dev/null || true
        # Pattern 2: CXXFLAGS='...'
        sed -i.tmp \"s/\\(CXXFLAGS='[^']*\\)'/\\1 -std=c++17'/\" \"$MEXOPTS_FILE\" 2>/dev/null || true
        # Pattern 3: CXXFLAGS=\" (at start)
        sed -i.tmp 's/^\\(CXXFLAGS=\"\\)/\\1-std=c++17 /' \"$MEXOPTS_FILE\" 2>/dev/null || true
        # Pattern 4: CXXFLAGS=' (at start)
        sed -i.tmp \"s/^\\(CXXFLAGS='\\)/\\1-std=c++17 /\" \"$MEXOPTS_FILE\" 2>/dev/null || true
        rm -f \"$MEXOPTS_FILE.tmp\" 2>/dev/null || true
    fi
fi

# Also set environment variables as backup
export CXXFLAGS=\"-std=c++17\"

# Call MEX
exec \"${MEX_EXECUTABLE}\" \"$@\"
")
    file(CHMOD ${WRAPPER_SCRIPT} FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif()

# Custom command to build MEX files
# Note: MEX doesn't accept compiler flags as direct arguments
# We need to pass C++17 standard via CXXFLAGS environment variable
# DUCC requires C++17 features (fold expressions, if constexpr, std::optional, etc.)
foreach(mex_file ${MEX_SOURCES})
    get_filename_component(mex_name ${mex_file} NAME_WE)
    
    # Determine output path
    set(MEX_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${mex_name}.${MEX_EXT}")
    
    # Create custom command to build MEX file
    # Pass C++17 standard via CXXFLAGS environment variable
    # This is required for DUCC which uses C++17 features
    # On Unix, use the wrapper script that sets CXXFLAGS
    if(UNIX)
        add_custom_command(
            OUTPUT ${MEX_OUTPUT}
            COMMAND ${WRAPPER_SCRIPT}
                -I${MATLAB_INCLUDE_DIR}
                -I${DUCC_SRC_DIR}
                -I${CMAKE_CURRENT_SOURCE_DIR}
                -cxx
                -outdir ${CMAKE_CURRENT_BINARY_DIR}
                ${mex_file}
                ${DUCC_SOURCES}
            DEPENDS ${mex_file} ${DUCC_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/ducc0_mex_utils.h ${WRAPPER_SCRIPT}
            COMMENT "Building MEX file ${mex_name}.${MEX_EXT}"
            VERBATIM
        )
    else()
        # Windows - use CMAKE_COMMAND -E env
        add_custom_command(
            OUTPUT ${MEX_OUTPUT}
            COMMAND ${CMAKE_COMMAND} -E env CXXFLAGS="-std=c++17"
                ${MEX_EXECUTABLE}
                -I${MATLAB_INCLUDE_DIR}
                -I${DUCC_SRC_DIR}
                -I${CMAKE_CURRENT_SOURCE_DIR}
                -cxx
                -outdir ${CMAKE_CURRENT_BINARY_DIR}
                ${mex_file}
                ${DUCC_SOURCES}
            DEPENDS ${mex_file} ${DUCC_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/ducc0_mex_utils.h
            COMMENT "Building MEX file ${mex_name}.${MEX_EXT}"
            VERBATIM
        )
    endif()
    
    # Create custom target
    add_custom_target(${mex_name}_mex ALL
        DEPENDS ${MEX_OUTPUT}
    )
endforeach()

# Installation
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/*.${MEX_EXT}
    DESTINATION lib/matlab/mex
    COMPONENT mex
)

# Print summary
message(STATUS "MATLAB root: ${MATLAB_ROOT}")
message(STATUS "MATLAB include: ${MATLAB_INCLUDE_DIR}")
message(STATUS "MEX compiler: ${MEX_EXECUTABLE}")
message(STATUS "MEX extension: ${MEX_EXT}")
message(STATUS "DUCC source: ${DUCC_SRC_DIR}")
message(STATUS "Building ${MEX_SOURCES}")

# Note: MEX_EXT will be determined at configure time
# For Windows: .mexw64 or .mexw32
# For Linux: .mexa64
# For Mac: .mexmaci64
