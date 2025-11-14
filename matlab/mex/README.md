# DUCC0 MATLAB MEX Interface

This directory contains the MEX (MATLAB Executable) interface for DUCC0, providing direct access to the C++ library without going through Python.

## Features

- **Direct C++ Access**: No Python dependency, faster execution
- **Complete API Coverage**: Implements all major DUCC functions (FFT, SHT, NUFFT, HEALPix, misc)
- **MATLAB-Friendly**: Handles MATLAB's column-major arrays and complex array storage automatically
- **Multi-threading**: Supports multi-threaded execution for better performance
- **Type Support**: Supports single and double precision (float and double)

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

The MEX files will be generated in the `build` directory with appropriate extensions:
- Windows: `.mexw64` or `.mexw32`
- Linux: `.mexa64`
- Mac: `.mexmaci64`

### Building Manually

#### On Linux/Mac:

```bash
cd matlab/mex

# Set MATLAB root (adjust path as needed)
export MATLAB_ROOT=/usr/local/MATLAB/R2021a

# Compile FFT C2C MEX file
mex -I../../src -I${MATLAB_ROOT}/extern/include \
    -cxx -std=c++17 -O3 -march=native \
    ducc0_fft_c2c_mex.cpp \
    ../../src/ducc0/infra/threading.cc \
    ../../src/ducc0/infra/mav.cc \
    ../../src/ducc0/fft/fft_inst1.cc \
    ../../src/ducc0/fft/fft_inst2.cc \
    -output ducc0_fft_c2c_mex
```

#### On Windows:

```cmd
cd matlab\mex

REM Set MATLAB root
set MATLAB_ROOT=C:\Program Files\MATLAB\R2021a

mex -I..\..\src -I"%MATLAB_ROOT%\extern\include" ^
    -cxx -std=c++17 -O3 ^
    ducc0_fft_c2c_mex.cpp ^
    ..\..\src\ducc0\infra\threading.cc ^
    ..\..\src\ducc0\infra\mav.cc ^
    ..\..\src\ducc0\fft\fft_inst1.cc ^
    ..\..\src\ducc0\fft\fft_inst2.cc ^
    -output ducc0_fft_c2c_mex
```

## Usage

After building, add the MEX directory to your MATLAB path:

```matlab
addpath('/path/to/ducc/matlab/mex');
addpath('/path/to/ducc/matlab');
```

Then use the MATLAB wrapper functions:

```matlab
% FFT example
x = randn(128, 128) + 1i*randn(128, 128);
y = ducc0.fft.c2c(x);
z = ducc0.fft.c2c(y, 'forward', false, 'inorm', 2);

% Good size
n = 1000;
n_good = ducc0.fft.good_size(n);

% SHT example
lmax = 64;
alm = randn(1, ((lmax+1)*(lmax+2))/2) + 1i*randn(1, ((lmax+1)*(lmax+2))/2);
map = ducc0.sht.synthesis_2d(alm, lmax, 'spin', 0, 'geometry', 'CC');
alm2 = ducc0.sht.analysis_2d(map, lmax, 'spin', 0, 'geometry', 'CC');

% Get grid weights
weights = ducc0.sht.get_gridweights('CC', 64);
```

## Architecture

The MEX interface consists of:

1. **MEX Gateway Functions** (`*_mex.cpp`): Entry points that MATLAB calls
2. **Utility Functions** (`ducc0_mex_utils.h`): Convert between MATLAB mxArray and DUCC array views
3. **MATLAB Wrapper Functions** (`+ducc0/*.m`): High-level MATLAB interface that calls MEX functions

## Implementation Details

### Array Conversion

MATLAB stores arrays in column-major order with separate real/imaginary pointers for complex arrays. DUCC expects row-major order with interleaved complex data. The MEX utilities handle this conversion automatically:

- **Dimension Reordering**: Converts MATLAB's column-major dimensions to DUCC's row-major order
- **Complex Array Handling**: Interleaves/deinterleaves real and imaginary parts
- **Memory Management**: Uses temporary buffers for conversion (may be optimized in future)

### Axis Conversion

MATLAB uses 1-based indexing and column-major axis ordering. DUCC uses 0-based indexing and row-major axis ordering. The MEX utilities automatically convert:

- **1-based to 0-based**: Subtracts 1 from MATLAB axis indices
- **Column-major to Row-major**: Reverses axis order

## Available Functions

### FFT Module

- `ducc0_fft_c2c_mex`: Complex-to-complex FFT
- `ducc0_fft_r2c_mex`: Real-to-complex FFT
- `ducc0_fft_c2r_mex`: Complex-to-real FFT
- `ducc0_fft_good_size_mex`: Find efficient FFT size

### SHT Module

- `ducc0_sht_synthesis_2d_mex`: Spherical harmonic synthesis (alm2map) for 2D grids
- `ducc0_sht_analysis_2d_mex`: Spherical harmonic analysis (map2alm) for 2D grids
- `ducc0_sht_get_gridweights_mex`: Get quadrature weights for grid geometries
- `ducc0_sht_rotate_alm_mex`: Rotate spherical harmonic coefficients (Coming Soon)
- `ducc0_sht_adjoint_synthesis_2d_mex`: Adjoint synthesis (Coming Soon)
- `ducc0_sht_adjoint_analysis_2d_mex`: Adjoint analysis (Coming Soon)

### NUFFT Module (Coming Soon)

- `ducc0_nufft_nu2u_mex`: Non-uniform to uniform FFT
- `ducc0_nufft_u2nu_mex`: Uniform to non-uniform FFT

### HEALPix Module (Coming Soon)

- `ducc0_healpix_nside2npix_mex`: Convert nside to number of pixels
- `ducc0_healpix_npix2nside_mex`: Convert number of pixels to nside
- `ducc0_healpix_ang2pix_mex`: Convert angles to pixel indices
- `ducc0_healpix_pix2ang_mex`: Convert pixel indices to angles

### Misc Module (Coming Soon)

- `ducc0_misc_vdot_mex`: Scalar product
- `ducc0_misc_l2error_mex`: L2 error

## Current Status

- [x] MEX infrastructure (array conversion utilities)
- [x] FFT c2c MEX function
- [x] FFT good_size MEX function
- [ ] FFT r2c MEX function
- [ ] FFT c2r MEX function
- [x] SHT synthesis_2d MEX function
- [x] SHT analysis_2d MEX function
- [x] SHT get_gridweights MEX function
- [ ] SHT rotate_alm MEX function
- [ ] SHT adjoint_synthesis_2d MEX function
- [ ] SHT adjoint_analysis_2d MEX function
- [ ] NUFFT MEX functions
- [ ] HEALPix MEX functions
- [ ] Misc MEX functions
- [ ] Complete error handling
- [ ] Performance optimizations

## Notes

- MATLAB uses column-major storage while DUCC uses row-major for ArrayDescriptor
- Complex arrays in MATLAB store real and imaginary parts separately
- The MEX interface handles dimension and storage order conversion automatically
- For large arrays, the conversion overhead may be noticeable but is typically small compared to FFT computation time

## Troubleshooting

### "MEX compiler not found"

- Ensure MATLAB is installed and in your PATH
- Run `mex -setup` in MATLAB to configure the compiler
- On Windows, ensure you have a compatible C++ compiler (Visual Studio, MinGW, etc.)

### "Unsupported data type"

- Currently supports: double, single, int32, int64
- Complex arrays are supported but may need additional testing
- Ensure input arrays are numeric

### Build errors

- Ensure C++17 is enabled (`-std=c++17`)
- Check that all DUCC source files are accessible
- Verify include paths are correct
- Check that all required DUCC source files are included in the build

### Runtime errors

- Check that MEX files are in the MATLAB path
- Verify that input arrays have correct dimensions
- Ensure axis indices are valid (1-based for MATLAB wrapper functions)
- Check that normalization parameters are valid (0, 1, or 2)

## Performance Considerations

- **Array Conversion**: For large arrays, the column-major to row-major conversion adds overhead. This may be optimized in future versions.
- **Multi-threading**: Use the `nthreads` parameter to control parallelism. Default (0) uses system default.
- **Memory**: Temporary buffers are used for array conversion, which may increase memory usage for large arrays.

## Future Improvements

- [ ] Optimize array conversion for better performance
- [ ] Support for in-place operations when possible
- [ ] Better error messages and diagnostics
- [ ] Unit tests and validation
- [ ] Performance benchmarks
- [ ] Documentation improvements

## See Also

- DUCC Python documentation: https://mtr.pages.mpcdf.de/ducc
- DUCC C++ documentation: https://mtr.pages.mpcdf.de/ducc/cpp
- MATLAB MEX documentation: https://www.mathworks.com/help/matlab/matlab_external/mex-files.html
