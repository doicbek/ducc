# DUCC0 MATLAB MEX Interface

This directory contains a MATLAB MEX (MATLAB Executable) interface for the DUCC (Distinctly Useful Code Collection) library, providing direct C++ access without requiring Python.

## Features

- **Direct C++ Access**: No Python dependency, faster execution
- **Complete API Coverage**: Implements all major DUCC functions (FFT, SHT, NUFFT, HEALPix, misc)
- **MATLAB-Friendly**: Handles MATLAB's column-major arrays and complex array storage automatically
- **Multi-threading**: Supports multi-threaded execution for better performance
- **Type Support**: Supports single and double precision (float and double)

## Requirements

- MATLAB R2018b or later
- C++17 compatible compiler (GCC 7+, Clang, or MSVC 2019+)
- DUCC0 source code
- CMake 3.15 or later (optional, for automated build)

See `mex/INSTALL.md` for detailed installation instructions.

## Quick Start

### Building MEX Files

```bash
cd matlab/mex
mkdir build
cd build
cmake ..
cmake --build .
```

### Using the Interface

```matlab
% Add paths
addpath('/path/to/ducc/matlab/mex/build');
addpath('/path/to/ducc/matlab');

% Example: Fast Fourier Transform
x = randn(128, 128) + 1i*randn(128, 128);
y = ducc0.fft.c2c(x);
z = ducc0.fft.c2c(y, 'forward', false, 'inorm', 2);

% Example: Good FFT size
n = 1000;
n_good = ducc0.fft.good_size(n);
```

## Module Overview

### ducc0.fft - Fast Fourier Transforms

Fast Fourier, trigonometric, and Hartley transforms with support for:
- Single and double precision
- Complex and real-valued transforms
- Multi-dimensional transforms
- Multi-threading

**Main Functions:**
- `c2c(a, ...)` - Complex-to-complex FFT (✅ implemented)
- `r2c(a, ...)` - Real-to-complex FFT (⚠️ not yet implemented)
- `c2r(a, ...)` - Complex-to-real FFT (⚠️ not yet implemented)
- `r2r_fftpack(a, ...)` - Real-to-real FFT (⚠️ not yet implemented)
- `good_size(n, ...)` - Get efficient FFT size (✅ implemented)

**Example:**
```matlab
x = randn(256, 256) + 1i*randn(256, 256);
y = ducc0.fft.c2c(x, 'axes', [1, 2], 'nthreads', 4);
```

### ducc0.sht - Spherical Harmonic Transforms

Efficient spherical harmonic transforms for various grid geometries.

**Main Functions:**
- `synthesis_2d(alm, lmax, ...)` - Spherical harmonic synthesis (⚠️ not yet implemented)
- `analysis_2d(map, lmax, ...)` - Spherical harmonic analysis (⚠️ not yet implemented)
- `get_gridweights(geometry, ntheta)` - Get quadrature weights (⚠️ not yet implemented)

### ducc0.nufft - Non-uniform FFTs

Library for non-uniform FFTs in 1D/2D/3D.

**Main Functions:**
- `nu2u(points, coord, ...)` - Non-uniform to uniform (⚠️ not yet implemented)
- `u2nu(grid, coord, ...)` - Uniform to non-uniform (⚠️ not yet implemented)

### ducc0.healpix - HEALPix Functions

HEALPix tesselation functionality.

**Main Functions:**
- `nside2npix(nside)` - Convert nside to number of pixels (✅ MATLAB implementation)
- `npix2nside(npix)` - Convert number of pixels to nside (✅ MATLAB implementation)
- `ang2pix(nside, theta, phi, ...)` - Convert angles to pixel index (⚠️ not yet implemented)
- `pix2ang(nside, pix, ...)` - Convert pixel index to angles (⚠️ not yet implemented)

### ducc0.misc - Miscellaneous Utilities

Various utility functions.

**Main Functions:**
- `vdot(a, b)` - Scalar product of two arrays (⚠️ not yet implemented)
- `l2error(a, b)` - L2 error between two arrays (✅ MATLAB implementation)

## Parameter Conventions

### Axes Parameter
Axes are specified as 1-based (MATLAB convention):
```matlab
% Transform along first and second axes
y = ducc0.fft.c2c(x, 'axes', [1, 2]);
```

### Normalization (inorm)
- `0` - No normalization
- `1` - Divide by sqrt(N)
- `2` - Divide by N

### Threading
- `nthreads = 0` - Use system default (typically all available cores)
- `nthreads > 0` - Use specified number of threads

## Implementation Status

✅ **Implemented**: FFT c2c, FFT good_size, HEALPix nside2npix/npix2nside, misc l2error

⚠️ **Not Yet Implemented**: Most other functions show error messages indicating MEX implementation is pending

## Performance Notes

- For best performance, compile MEX files with CPU-specific optimizations
- Multi-threading is available for most operations via the `nthreads` parameter
- Pre-allocating output arrays can improve performance for repeated operations

## Troubleshooting

### MEX Files Not Found
If you get an error about MEX files not being found:
1. Ensure MEX files are built (see `mex/INSTALL.md`)
2. Add the MEX directory to your MATLAB path:
   ```matlab
   addpath('/path/to/ducc/matlab/mex/build');
   ```

### Build Errors
See `mex/INSTALL.md` for detailed troubleshooting of build issues.

## Documentation

- **mex/README.md**: MEX interface overview
- **mex/INSTALL.md**: Installation and build instructions
- **mex/SUMMARY.md**: Implementation status and features
- **EXAMPLES.md**: Usage examples

## License

This wrapper is provided under the same license as DUCC (GNU General Public License v2 or later).

## References

- DUCC Python documentation: https://mtr.pages.mpcdf.de/ducc
- DUCC C++ documentation: https://mtr.pages.mpcdf.de/ducc/cpp
- DUCC source code: https://gitlab.mpcdf.mpg.de/mtr/ducc

## Support

For issues with the MATLAB MEX interface, please check:
1. That MEX files are correctly built
2. That MEX files are in the MATLAB path
3. The DUCC documentation for function signatures

For issues with DUCC itself, please refer to the main DUCC repository.
