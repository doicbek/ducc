# Building DUCC0 MATLAB MEX Interface

## Quick Start

### Option 1: Using MATLAB's mex command (Simplest)

```matlab
% In MATLAB, navigate to the mex directory
cd matlab/mex

% Set up paths
ducc_src = '../../src';
matlab_inc = fullfile(matlabroot, 'extern', 'include');

% Build FFT MEX file
mex('-I', ducc_src, '-I', matlab_inc, ...
    '-cxx', '-std=c++17', '-O3', ...
    'ducc0_fft_mex.cpp', ...
    fullfile(ducc_src, 'ducc0/infra/threading.cc'), ...
    fullfile(ducc_src, 'ducc0/infra/mav.cc'), ...
    fullfile(ducc_src, 'ducc0/fft/fft_inst1.cc'), ...
    fullfile(ducc_src, 'ducc0/fft/fft_inst2.cc'), ...
    '-output', 'ducc0_fft_mex');
```

### Option 2: Using CMake

```bash
cd matlab/mex
mkdir build && cd build
cmake ..
make
```

### Option 3: Manual Compilation

See the platform-specific instructions below.

## Platform-Specific Instructions

### Linux

```bash
cd matlab/mex

# Set MATLAB root (adjust for your installation)
export MATLAB_ROOT=/usr/local/MATLAB/R2021a

# Compile
g++ -shared -fPIC -std=c++17 -O3 -march=native \
    -I../../src \
    -I${MATLAB_ROOT}/extern/include \
    -o ducc0_fft_mex.mexa64 \
    ducc0_fft_mex.cpp \
    ../../src/ducc0/infra/threading.cc \
    ../../src/ducc0/infra/mav.cc \
    ../../src/ducc0/fft/fft_inst1.cc \
    ../../src/ducc0/fft/fft_inst2.cc \
    -pthread
```

### macOS

```bash
cd matlab/mex

export MATLAB_ROOT=/Applications/MATLAB_R2021a.app

# Compile
clang++ -shared -fPIC -std=c++17 -O3 \
    -I../../src \
    -I${MATLAB_ROOT}/extern/include \
    -o ducc0_fft_mex.mexmaci64 \
    ducc0_fft_mex.cpp \
    ../../src/ducc0/infra/threading.cc \
    ../../src/ducc0/infra/mav.cc \
    ../../src/ducc0/fft/fft_inst1.cc \
    ../../src/ducc0/fft/fft_inst2.cc \
    -pthread
```

### Windows (MSVC)

```cmd
cd matlab\mex

set MATLAB_ROOT=C:\Program Files\MATLAB\R2021a

cl /LD /EHsc /std:c++17 /O2 ^
    /I..\..\src ^
    /I"%MATLAB_ROOT%\extern\include" ^
    ducc0_fft_mex.cpp ^
    ..\..\src\ducc0\infra\threading.cc ^
    ..\..\src\ducc0\infra\mav.cc ^
    ..\..\src\ducc0\fft\fft_inst1.cc ^
    ..\..\src\ducc0\fft\fft_inst2.cc ^
    /Fe:ducc0_fft_mex.mexw64
```

## Required Source Files

The MEX interface requires these DUCC source files:

- `ducc0/infra/threading.cc`
- `ducc0/infra/mav.cc`
- `ducc0/fft/fft_inst1.cc`
- `ducc0/fft/fft_inst2.cc`
- Plus all header files in the include path

## Troubleshooting

### Compiler Not Found
- Run `mex -setup` in MATLAB to configure your compiler
- Ensure C++17 support is available

### Link Errors
- Ensure all required DUCC source files are included
- Check that include paths are correct
- Verify MATLAB library paths are set

### Runtime Errors
- Ensure MEX files are in MATLAB path
- Check MATLAB version compatibility
- Verify array dimensions and types match expectations



