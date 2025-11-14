# DUCC0 MATLAB MEX Interface - Summary

## Overview

This MATLAB MEX interface provides direct access to the DUCC (Distinctly Useful Code Collection) library through MATLAB's MEX (MATLAB Executable) interface. The wrapper mirrors the Python API structure, making it easy to use DUCC's efficient numerical computation algorithms from MATLAB without requiring Python.

## Structure

```
matlab/
├── +ducc0/              % Main package namespace
│   ├── ducc0.m          % Main module entry point
│   ├── +fft/            % Fast Fourier Transform module
│   │   ├── c2c.m        % Complex-to-complex FFT (✅ MEX)
│   │   ├── r2c.m        % Real-to-complex FFT (⚠️ pending)
│   │   ├── c2r.m        % Complex-to-real FFT (⚠️ pending)
│   │   ├── r2r_fftpack.m % Real-to-real FFT (⚠️ pending)
│   │   └── good_size.m  % Find efficient FFT size (✅ MEX)
│   ├── +sht/            % Spherical Harmonic Transform module
│   │   ├── synthesis_2d.m (✅ MEX)
│   │   ├── analysis_2d.m (✅ MEX)
│   │   ├── adjoint_synthesis_2d.m (⚠️ pending)
│   │   ├── adjoint_analysis_2d.m (⚠️ pending)
│   │   └── get_gridweights.m (✅ MEX)
│   ├── +nufft/          % Non-uniform FFT module
│   │   ├── nu2u.m       % Non-uniform to uniform (⚠️ pending)
│   │   └── u2nu.m       % Uniform to non-uniform (⚠️ pending)
│   ├── +healpix/        % HEALPix module
│   │   ├── nside2npix.m (✅ MATLAB)
│   │   ├── npix2nside.m (✅ MATLAB)
│   │   ├── ang2pix.m (⚠️ pending)
│   │   └── pix2ang.m (⚠️ pending)
│   ├── +misc/           % Miscellaneous utilities
│   │   ├── vdot.m       % Scalar product (⚠️ pending)
│   │   └── l2error.m    % L2 error (✅ MATLAB)
│   └── +util/           % Internal utilities (deprecated)
│       ├── matlab2numpy.m  % Deprecated (no longer needed)
│       └── numpy2matlab.m   % Deprecated (no longer needed)
├── mex/                 % MEX interface source files
│   ├── ducc0_fft_c2c_mex.cpp (✅ implemented)
│   ├── ducc0_fft_good_size_mex.cpp (✅ implemented)
│   ├── ducc0_sht_synthesis_2d_mex.cpp (✅ implemented)
│   ├── ducc0_sht_analysis_2d_mex.cpp (✅ implemented)
│   ├── ducc0_sht_get_gridweights_mex.cpp (✅ implemented)
│   ├── ducc0_mex_utils.h (✅ implemented)
│   └── CMakeLists.txt (✅ build system)
├── README.md            % Main documentation
├── INSTALL.md           % Installation guide
├── EXAMPLES.md          % Usage examples
└── SUMMARY.md           % This file
```

## Key Features

### 1. MEX Interface
- Direct C++ access without Python dependency
- Handles MATLAB's column-major arrays automatically
- Converts between MATLAB and DUCC array formats
- Supports multi-threading

### 2. API Compatibility
- Mirrors Python API structure and function signatures
- Uses MATLAB-style parameter parsing with `inputParser`
- Handles optional parameters with name-value pairs
- 1-based axis indexing (MATLAB convention)

### 3. Data Type Conversion
- Automatic conversion between MATLAB and DUCC arrays
- Handles real and complex arrays
- Converts between column-major (MATLAB) and row-major (DUCC) ordering
- Supports single and double precision

### 4. Module Coverage
- **FFT**: Complex FFT (✅), good_size (✅), others pending
- **SHT**: synthesis_2d (✅), analysis_2d (✅), get_gridweights (✅), others pending
- **NUFFT**: All functions pending MEX implementation
- **HEALPix**: Simple functions (✅), others pending
- **Misc**: l2error (✅), vdot pending

## Implementation Status

### ✅ Implemented
- FFT c2c (MEX)
- FFT good_size (MEX)
- SHT synthesis_2d (MEX)
- SHT analysis_2d (MEX)
- SHT get_gridweights (MEX)
- HEALPix nside2npix (MATLAB)
- HEALPix npix2nside (MATLAB)
- Misc l2error (MATLAB)

### ⚠️ Pending MEX Implementation
- FFT r2c, c2r, r2r_fftpack
- SHT rotate_alm, adjoint_synthesis_2d, adjoint_analysis_2d
- NUFFT nu2u, u2nu
- HEALPix ang2pix, pix2ang
- Misc vdot

Functions that are not yet implemented show error messages indicating MEX implementation is pending.

## Usage Pattern

```matlab
% Add paths
addpath('/path/to/ducc/matlab/mex/build');
addpath('/path/to/ducc/matlab');

% Use functions with MATLAB-style syntax
result = ducc0.fft.c2c(data, 'axes', [1, 2], 'nthreads', 4);
```

## Design Decisions

1. **MEX Interface**: Uses MATLAB's MEX interface for direct C++ access, no Python dependency
2. **Package Structure**: Uses MATLAB's package namespace (`+ducc0`) to mirror Python's module structure
3. **Parameter Handling**: Uses `inputParser` for MATLAB-friendly parameter handling
4. **Error Handling**: Provides clear error messages for unimplemented functions
5. **Array Conversion**: Handles MATLAB's column-major storage vs DUCC's row-major expectations

## Limitations

1. **Incomplete Implementation**: Many functions are not yet implemented via MEX
2. **Array Conversion**: Current implementation uses temporary buffers, which adds overhead
3. **Type Support**: Currently supports double and single precision only
4. **Advanced Features**: Some advanced features may not be directly accessible

## Future Enhancements

1. Implement remaining MEX functions (r2c, c2r, SHT, NUFFT, HEALPix, misc)
2. Optimize array conversion for better performance
3. Support for in-place operations when possible
4. Better error messages and diagnostics
5. Unit tests and validation
6. Performance benchmarks

## Testing

To test the installation:
```matlab
% Basic FFT test
x = randn(64, 64) + 1i*randn(64, 64);
y = ducc0.fft.c2c(x);
z = ducc0.fft.c2c(y, 'forward', false, 'inorm', 2);
fprintf('Error: %e\n', max(abs(x(:) - z(:))));
```

## Support

- See `mex/README.md` for usage documentation
- See `mex/INSTALL.md` for installation help
- See `EXAMPLES.md` for code examples
- Refer to DUCC C++ documentation: https://mtr.pages.mpcdf.de/ducc/cpp
