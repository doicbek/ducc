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

See `mex/INSTALL.md` for detailed manual build instructions.

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

See `mex/INSTALL.md` for detailed troubleshooting information.

## Next Steps

- See `mex/README.md` for usage examples
- See `EXAMPLES.md` for more examples
- Check DUCC C++ documentation: https://mtr.pages.mpcdf.de/ducc/cpp

## Support

- Check `mex/README.md` for common issues
- Report issues on the DUCC GitLab repository
- Consult DUCC documentation: https://mtr.pages.mpcdf.de/ducc
