# DUCC0 MATLAB MEX Interface - Installation Guide

This guide explains how to install and build the DUCC0 MATLAB MEX interface.

## Prerequisites

### Required Software

1. **MATLAB** (R2018b or later)
   - Download from: https://www.mathworks.com/products/matlab.html
   - Ensure MATLAB is in your PATH or set `MATLAB_ROOT` environment variable

2. **C++ Compiler** (C++17 compatible)
   - **Windows**: Visual Studio 2019 or later, or MinGW-w64
   - **Linux**: GCC 7+ or Clang 6+
   - **Mac**: Xcode Command Line Tools (Clang)

3. **CMake** (3.15 or later, optional but recommended)
   - Download from: https://cmake.org/download/
   - Or use package manager: `apt install cmake` (Linux), `brew install cmake` (Mac)

### Optional Software

- **Git** (for cloning the repository)
- **Make** or **Ninja** (for building)

## Installation Methods

### Method 1: Build with CMake (Recommended)

1. **Clone the repository** (if not already done):
   ```bash
   git clone https://gitlab.mpcdf.mpg.de/mtr/ducc.git
   cd ducc/matlab/mex
   ```

2. **Create build directory**:
   ```bash
   mkdir build
   cd build
   ```

3. **Configure with CMake**:
   ```bash
   cmake ..
   ```

   If MATLAB is not found automatically, specify the path:
   ```bash
   cmake -DMATLAB_ROOT=/path/to/matlab ..
   ```

   Or set the environment variable:
   ```bash
   export MATLAB_ROOT=/path/to/matlab
   cmake ..
   ```

4. **Build**:
   ```bash
   cmake --build .
   ```

   Or use make:
   ```bash
   make
   ```

5. **Install** (optional):
   ```bash
   cmake --install .
   ```

### Method 2: Build Manually

#### On Linux/Mac:

1. **Set MATLAB root**:
   ```bash
   export MATLAB_ROOT=/usr/local/MATLAB/R2021a
   # Or wherever MATLAB is installed
   ```

2. **Navigate to MEX directory**:
   ```bash
   cd ducc/matlab/mex
   ```

3. **Build MEX files**:
   ```bash
   # Build FFT C2C MEX file
   mex -I../../src -I${MATLAB_ROOT}/extern/include \
       -cxx -std=c++17 -O3 -march=native \
       ducc0_fft_c2c_mex.cpp \
       ../../src/ducc0/infra/threading.cc \
       ../../src/ducc0/infra/mav.cc \
       ../../src/ducc0/fft/fft_inst1.cc \
       ../../src/ducc0/fft/fft_inst2.cc \
       -output ducc0_fft_c2c_mex
   
   # Build FFT good_size MEX file
   mex -I../../src -I${MATLAB_ROOT}/extern/include \
       -cxx -std=c++17 -O3 \
       ducc0_fft_good_size_mex.cpp \
       ../../src/ducc0/fft/fft1d_impl.cc \
       -output ducc0_fft_good_size_mex
   ```

#### On Windows:

1. **Set MATLAB root**:
   ```cmd
   set MATLAB_ROOT=C:\Program Files\MATLAB\R2021a
   ```

2. **Navigate to MEX directory**:
   ```cmd
   cd ducc\matlab\mex
   ```

3. **Build MEX files**:
   ```cmd
   mex -I..\..\src -I"%MATLAB_ROOT%\extern\include" ^
       -cxx -std=c++17 -O2 ^
       ducc0_fft_c2c_mex.cpp ^
       ..\..\src\ducc0\infra\threading.cc ^
       ..\..\src\ducc0\infra\mav.cc ^
       ..\..\src\ducc0\fft\fft_inst1.cc ^
       ..\..\src\ducc0\fft\fft_inst2.cc ^
       -output ducc0_fft_c2c_mex
   
   mex -I..\..\src -I"%MATLAB_ROOT%\extern\include" ^
       -cxx -std=c++17 -O2 ^
       ducc0_fft_good_size_mex.cpp ^
       ..\..\src\ducc0\fft\fft1d_impl.cc ^
       -output ducc0_fft_good_size_mex
   ```

## Configuration

### Setting MATLAB Path

After building, add the MEX directory to your MATLAB path:

```matlab
% Add MEX directory
addpath('/path/to/ducc/matlab/mex/build');

% Add MATLAB wrapper directory
addpath('/path/to/ducc/matlab');
```

Or add permanently to your `startup.m`:

```matlab
% Add to startup.m
addpath('/path/to/ducc/matlab/mex/build');
addpath('/path/to/ducc/matlab');
```

### Configuring MEX Compiler

If MEX compiler is not configured:

1. **Run MATLAB**:
   ```matlab
   mex -setup
   ```

2. **Select C++ compiler**:
   - Choose from available compilers
   - Follow on-screen instructions

## Verification

### Test Installation

1. **Start MATLAB**:
   ```matlab
   matlab
   ```

2. **Test FFT function**:
   ```matlab
   % Test basic FFT
   x = randn(64, 64) + 1i*randn(64, 64);
   y = ducc0.fft.c2c(x);
   z = ducc0.fft.c2c(y, 'forward', false, 'inorm', 2);
   fprintf('Error: %e\n', max(abs(x(:) - z(:))));
   ```

3. **Test good_size function**:
   ```matlab
   n = 1000;
   n_good = ducc0.fft.good_size(n);
   fprintf('Good size for %d: %d\n', n, n_good);
   ```

### Expected Output

- FFT test should show error < 1e-10 (numerical precision)
- good_size should return a value >= input value

## Troubleshooting

### "MATLAB not found"

**Solution**: Set `MATLAB_ROOT` environment variable:
```bash
export MATLAB_ROOT=/path/to/matlab
```

### "MEX compiler not found"

**Solution**: 
1. Ensure MATLAB is in PATH
2. Run `mex -setup` in MATLAB
3. Check that MEX compiler is available:
   ```matlab
   mex -setup C++
   ```

### "C++17 not supported"

**Solution**: 
1. Update compiler to a C++17-compatible version
2. Check compiler version:
   ```bash
   g++ --version  # Should be 7 or later
   clang++ --version  # Should be 6 or later
   ```

### "Duplicate symbols" or "Link errors"

**Solution**: 
1. Ensure all DUCC source files are included
2. Check that source files are not included multiple times
3. Verify include paths are correct

### "Unsupported data type"

**Solution**: 
1. Ensure input arrays are numeric (double or single)
2. Check that complex arrays are properly formatted
3. Verify array dimensions are correct

### Build takes too long

**Solution**: 
1. Use release build configuration
2. Reduce optimization level (if debugging)
3. Use parallel build: `cmake --build . -j$(nproc)`

## Advanced Configuration

### Custom Compiler Flags

Edit `CMakeLists.txt` to add custom flags:
```cmake
set(COMPILER_FLAGS "${COMPILER_FLAGS} -mavx2")
```

### Debug Build

Build with debug symbols:
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

### Release Build

Build with optimizations:
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

## Next Steps

- See [README.md](README.md) for usage examples
- See [EXAMPLES.md](../EXAMPLES.md) for more examples
- Check DUCC Python documentation: https://mtr.pages.mpcdf.de/ducc

## Support

- Check [README.md](README.md) for common issues
- Report issues on the DUCC GitLab repository
- Consult DUCC documentation: https://mtr.pages.mpcdf.de/ducc

