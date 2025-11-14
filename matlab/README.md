# DUCC0 MATLAB Wrapper

This directory contains a MATLAB wrapper for the DUCC (Distinctly Useful Code Collection) library, providing access to efficient algorithms for Fast Fourier Transforms, Spherical Harmonic Transforms, non-uniform FFTs, and other numerical computation tools.

## Two Interface Options

This wrapper provides **two ways** to access DUCC:

1. **Python Interface** (Default): Uses MATLAB's Python interface to call the ducc0 Python package
   - Easy to set up (just install Python package)
   - Full API coverage
   - Some overhead from Python interface

2. **MEX Interface** (Recommended for Performance): Direct C++ access via MEX files
   - Better performance (no Python overhead)
   - No Python dependency
   - Requires compilation
   - See `README_MEX.md` for details

## Requirements

### For Python Interface:
- MATLAB R2018b or later (with Python interface support)
- Python 3.8 or later
- ducc0 Python package installed: `pip install ducc0`

### For MEX Interface:
- MATLAB R2018b or later
- C++17 compatible compiler (GCC 7+, Clang, or MSVC 2019+)
- DUCC0 source code
- See `mex/BUILD.md` for build instructions

## Installation

1. Ensure ducc0 is installed in your Python environment:
   ```bash
   pip install ducc0
   ```

2. Configure MATLAB to use the correct Python interpreter:
   ```matlab
   pyversion  % Check current Python version
   pyversion('/path/to/python')  % Set if needed
   ```

3. Add the matlab directory to your MATLAB path:
   ```matlab
   addpath('/path/to/ducc/matlab')
   ```

## Quick Start

### Using Python Interface (Default)

```matlab
% Import the module
import ducc0.*

% Example: Fast Fourier Transform
x = randn(128, 128) + 1i*randn(128, 128);
y = ducc0.fft.c2c(x);
z = ducc0.fft.c2c(y, 'forward', false);  % Inverse transform

% Example: Real-to-complex FFT
x_real = randn(128);
y_complex = ducc0.fft.r2c(x_real);

% Example: Spherical Harmonic Transform
lmax = 32;
alm = randn(1, (lmax+1)*(lmax+2)/2) + 1i*randn(1, (lmax+1)*(lmax+2)/2);
map = ducc0.sht.synthesis_2d(alm, lmax, 'ntheta', 33, 'nphi', 66);
alm2 = ducc0.sht.analysis_2d(map, lmax);
```

### Using MEX Interface (Better Performance)

```matlab
% Build MEX files first (see mex/BUILD.md)
% Then use _mex versions of functions:

x = randn(128, 128) + 1i*randn(128, 128);
y = ducc0.fft.c2c_mex(x, 'nthreads', 4);  % Direct C++ access
```

## Module Overview

### ducc0.fft - Fast Fourier Transforms

Fast Fourier, trigonometric, and Hartley transforms with support for:
- Single, double, and long double precision
- Complex and real-valued transforms
- Multi-dimensional transforms
- Multi-threading

**Main Functions:**
- `c2c(a, ...)` - Complex-to-complex FFT
- `r2c(a, ...)` - Real-to-complex FFT
- `c2r(a, ...)` - Complex-to-real FFT
- `r2r_fftpack(a, ...)` - Real-to-real FFT (FFTPACK convention)
- `good_size(n, ...)` - Get efficient FFT size

**Example:**
```matlab
x = randn(256, 256);
y = ducc0.fft.r2c(x, 'axes', [0, 1], 'nthreads', 4);
```

### ducc0.sht - Spherical Harmonic Transforms

Efficient spherical harmonic transforms for various grid geometries.

**Main Functions:**
- `synthesis_2d(alm, lmax, ...)` - Spherical harmonic synthesis (alm2map)
- `analysis_2d(map, lmax, ...)` - Spherical harmonic analysis (map2alm)
- `adjoint_synthesis_2d(alm, lmax, ...)` - Adjoint synthesis
- `adjoint_analysis_2d(map, lmax, ...)` - Adjoint analysis
- `get_gridweights(geometry, ntheta)` - Get quadrature weights
- `rotate_alm(alm, lmax, psi, theta, phi, ...)` - Rotate spherical harmonics

**Supported Geometries:**
- `'CC'` - Clenshaw-Curtis
- `'F1'` - Fejer's first rule
- `'MW'` - McEwen & Wiaux
- `'MWflip'` - Flipped McEwen & Wiaux
- `'GL'` - Gauss-Legendre
- `'DH'` - Driscoll-Healy
- `'F2'` - Fejer's second rule

**Example:**
```matlab
lmax = 64;
mmax = 64;
alm = randn(1, (lmax+1)*(lmax+2)/2) + 1i*randn(1, (lmax+1)*(lmax+2)/2);
map = ducc0.sht.synthesis_2d(alm, lmax, 'mmax', mmax, ...
    'ntheta', lmax+1, 'nphi', 2*mmax+2, 'geometry', 'CC');
```

### ducc0.nufft - Non-uniform FFTs

Library for non-uniform FFTs in 1D/2D/3D.

**Main Functions:**
- `nu2u(points, coord, ...)` - Non-uniform to uniform (points to grid)
- `u2nu(grid, coord, ...)` - Uniform to non-uniform (grid to points)
- `nu2nu(points_in, coord_in, coord_out, ...)` - Non-uniform to non-uniform

**Example:**
```matlab
% Generate random non-uniform points
npoints = 1000;
points = randn(npoints, 1) + 1i*randn(npoints, 1);
coord = randn(npoints, 1) * 2*pi;  % 1D coordinates

% Transform to uniform grid
grid = ducc0.nufft.nu2u(points, coord, 'grid_shape', [256], ...
    'epsilon', 1e-12, 'nthreads', 4);
```

### ducc0.healpix - HEALPix Functions

Python bindings for HEALPix tesselation functionality.

**Main Functions:**
- `nside2npix(nside)` - Convert nside to number of pixels
- `npix2nside(npix)` - Convert number of pixels to nside
- `ang2pix(nside, theta, phi, ...)` - Convert angles to pixel index
- `pix2ang(nside, pix, ...)` - Convert pixel index to angles
- `pix2vec(nside, pix, ...)` - Convert pixel index to unit vector
- `vec2pix(nside, vec, ...)` - Convert unit vector to pixel index

**Example:**
```matlab
nside = 256;
theta = pi/3;  % Colatitude
phi = pi/4;    % Azimuth
pix = ducc0.healpix.ang2pix(nside, theta, phi);
[theta2, phi2] = ducc0.healpix.pix2ang(nside, pix);
```

### ducc0.misc - Miscellaneous Utilities

Various utility functions.

**Main Functions:**
- `vdot(a, b)` - Scalar product of two arrays
- `l2error(a, b)` - L2 error between two arrays

**Example:**
```matlab
a = randn(100, 100);
b = randn(100, 100);
dot_product = ducc0.misc.vdot(a, b);
error = ducc0.misc.l2error(a, b);
```

## Parameter Conventions

### Axes Parameter
Axes are specified as 0-indexed (matching Python convention):
```matlab
% Transform along first and second axes
y = ducc0.fft.c2c(x, 'axes', [0, 1]);
```

### Normalization (inorm)
- `0` - No normalization
- `1` - Divide by sqrt(N)
- `2` - Divide by N

### Threading
- `nthreads = 0` - Use system default (typically all available cores)
- `nthreads > 0` - Use specified number of threads

## Data Type Handling

The wrapper automatically handles conversion between MATLAB and Python/NumPy arrays:
- MATLAB `double` → NumPy `float64`
- MATLAB `single` → NumPy `float32`
- MATLAB `complex` → NumPy `complex128`
- MATLAB `int32` → NumPy `int32`
- MATLAB `int64` → NumPy `int64`

Note: Python uses row-major ordering while MATLAB uses column-major. The wrapper handles this conversion automatically.

## Performance Notes

- For best performance, compile ducc0 from source with CPU-specific optimizations
- Multi-threading is available for most operations via the `nthreads` parameter
- Pre-allocating output arrays can improve performance for repeated operations

## Troubleshooting

### Python Module Not Found
If you get an error about the Python module not being found:
```matlab
% Check Python version
pyversion

% Verify ducc0 is installed
py.importlib.import_module('ducc0')
```

### Array Conversion Issues
If you encounter issues with array conversion, try:
```matlab
% Ensure arrays are in the correct format
x = double(x);  % Convert to double if needed
x = complex(x);  % Ensure complex arrays are properly formatted
```

## License

This wrapper is provided under the same license as DUCC (GNU General Public License v2 or later).

## References

- DUCC Python documentation: https://mtr.pages.mpcdf.de/ducc
- DUCC source code: https://gitlab.mpcdf.mpg.de/mtr/ducc

## Support

For issues with the MATLAB wrapper, please check:
1. That ducc0 Python package is correctly installed
2. That MATLAB can access the Python interpreter
3. The DUCC Python documentation for function signatures

For issues with DUCC itself, please refer to the main DUCC repository.

