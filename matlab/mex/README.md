# DUCC0 MATLAB MEX Interface

This directory contains the MEX (MATLAB Executable) interface for DUCC0, providing direct access to the C++ library without going through Python.

## Building

### Prerequisites

- MATLAB (R2018b or later)
- C++17 compatible compiler (GCC 7+, Clang, or MSVC 2019+)
- CMake 3.15 or later (optional, for automated build)
- DUCC0 source code

### Building with CMake (Recommended)

```bash
cd matlab/mex
mkdir build
cd build
cmake ..
cmake --build .
```

The MEX files will be generated in the `build` directory.

### Building Manually

#### On Linux/Mac:

```bash
cd matlab/mex

# Set MATLAB root (adjust path as needed)
export MATLAB_ROOT=/usr/local/MATLAB/R2021a

# Compile FFT MEX file
mex -I../../src -I${MATLAB_ROOT}/extern/include \
    -cxx -std=c++17 -O3 -march=native \
    ducc0_fft_mex.cpp \
    ../../src/ducc0/infra/threading.cc \
    ../../src/ducc0/infra/mav.cc \
    ../../src/ducc0/fft/fft_inst1.cc \
    ../../src/ducc0/fft/fft_inst2.cc \
    -output ducc0_fft_mex
```

#### On Windows:

```cmd
cd matlab\mex

REM Set MATLAB root
set MATLAB_ROOT=C:\Program Files\MATLAB\R2021a

mex -I..\..\src -I"%MATLAB_ROOT%\extern\include" ^
    -cxx -std=c++17 -O3 ^
    ducc0_fft_mex.cpp ^
    ..\..\src\ducc0\infra\threading.cc ^
    ..\..\src\ducc0\infra\mav.cc ^
    ..\..\src\ducc0\fft\fft_inst1.cc ^
    ..\..\src\ducc0\fft\fft_inst2.cc ^
    -output ducc0_fft_mex
```

## Usage

After building, add the MEX directory to your MATLAB path:

```matlab
addpath('/path/to/ducc/matlab/mex');
```

Then use the MEX functions directly or through the MATLAB wrapper functions in `+ducc0/`.

## Architecture

The MEX interface consists of:

1. **MEX Gateway Functions** (`*_mex.cpp`): Entry points that MATLAB calls
2. **Utility Functions** (`ducc0_mex_utils.h`): Convert between MATLAB mxArray and DUCC ArrayDescriptor
3. **MATLAB Wrapper Functions** (`+ducc0/*.m`): High-level MATLAB interface that calls MEX functions

## Current Status

- [x] MEX infrastructure (array conversion utilities)
- [x] FFT c2c MEX function (basic implementation)
- [x] FFT r2c MEX function (basic implementation)
- [ ] FFT c2r MEX function
- [ ] SHT MEX functions
- [ ] NUFFT MEX functions
- [ ] HEALPix MEX functions
- [ ] Complete error handling
- [ ] Complex array handling improvements

## Notes

- MATLAB uses column-major storage while DUCC uses row-major for ArrayDescriptor
- Complex arrays in MATLAB store real and imaginary parts separately
- The MEX interface handles dimension and storage order conversion automatically

## Troubleshooting

### "MEX compiler not found"
- Ensure MATLAB is installed and in your PATH
- Run `mex -setup` in MATLAB to configure the compiler

### "Unsupported data type"
- Currently supports: double, single, int32, int64
- Complex arrays are supported but may need additional testing

### Build errors
- Ensure C++17 is enabled
- Check that all DUCC source files are accessible
- Verify include paths are correct

