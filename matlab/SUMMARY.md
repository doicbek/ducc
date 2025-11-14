# DUCC0 MATLAB Wrapper - Summary

## Overview

This MATLAB wrapper provides access to the DUCC (Distinctly Useful Code Collection) library through MATLAB's Python interface. The wrapper mirrors the Python API structure, making it easy to use DUCC's efficient numerical computation algorithms from MATLAB.

## Structure

```
matlab/
├── +ducc0/              % Main package namespace
│   ├── ducc0.m          % Main module entry point
│   ├── +fft/            % Fast Fourier Transform module
│   │   ├── c2c.m        % Complex-to-complex FFT
│   │   ├── r2c.m        % Real-to-complex FFT
│   │   ├── c2r.m        % Complex-to-real FFT
│   │   ├── r2r_fftpack.m % Real-to-real FFT (FFTPACK)
│   │   └── good_size.m  % Find efficient FFT size
│   ├── +sht/            % Spherical Harmonic Transform module
│   │   ├── synthesis_2d.m
│   │   ├── analysis_2d.m
│   │   ├── adjoint_synthesis_2d.m
│   │   ├── adjoint_analysis_2d.m
│   │   └── get_gridweights.m
│   ├── +nufft/          % Non-uniform FFT module
│   │   ├── nu2u.m       % Non-uniform to uniform
│   │   └── u2nu.m       % Uniform to non-uniform
│   ├── +healpix/        % HEALPix module
│   │   ├── nside2npix.m
│   │   ├── npix2nside.m
│   │   ├── ang2pix.m
│   │   └── pix2ang.m
│   ├── +misc/           % Miscellaneous utilities
│   │   ├── vdot.m       % Scalar product
│   │   └── l2error.m    % L2 error
│   └── +util/           % Internal utilities
│       ├── matlab2numpy.m  % Convert MATLAB to NumPy
│       └── numpy2matlab.m   % Convert NumPy to MATLAB
├── README.md            % Main documentation
├── INSTALL.md           % Installation guide
├── EXAMPLES.md          % Usage examples
└── SUMMARY.md           % This file
```

## Key Features

### 1. API Compatibility
- Mirrors Python API structure and function signatures
- Uses MATLAB-style parameter parsing with `inputParser`
- Handles optional parameters with name-value pairs

### 2. Data Type Conversion
- Automatic conversion between MATLAB and NumPy arrays
- Handles real and complex arrays
- Converts between row-major (Python) and column-major (MATLAB) ordering

### 3. Module Coverage
- **FFT**: Complex and real FFTs, multi-dimensional transforms
- **SHT**: Spherical harmonic synthesis and analysis
- **NUFFT**: Non-uniform FFTs in 1D/2D/3D
- **HEALPix**: Pixel operations, coordinate conversions
- **Misc**: Utility functions (vdot, l2error)

### 4. Documentation
- Comprehensive README with quick start guide
- Detailed installation instructions
- Extensive examples for all modules
- Inline help for all functions

## Usage Pattern

```matlab
% Import module
import ducc0.*

% Use functions with MATLAB-style syntax
result = ducc0.fft.c2c(data, 'axes', [0, 1], 'nthreads', 4);
```

## Design Decisions

1. **Python Interface**: Uses MATLAB's built-in Python interface rather than MEX files for easier maintenance and compatibility
2. **Package Structure**: Uses MATLAB's package namespace (`+ducc0`) to mirror Python's module structure
3. **Parameter Handling**: Uses `inputParser` for MATLAB-friendly parameter handling while maintaining Python API compatibility
4. **Error Handling**: Provides clear error messages and fallback conversion methods

## Limitations

1. **Performance**: Python interface has some overhead compared to native MEX files, but is typically negligible for large arrays
2. **Type Support**: Limited to numeric types (real and complex arrays)
3. **Advanced Features**: Some advanced Python features may not be directly accessible

## Future Enhancements

Potential improvements:
- Additional wrapper functions for wgridder and totalconvolve modules
- More comprehensive error handling
- Performance optimizations for array conversion
- Additional utility functions
- Unit tests

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

- See README.md for usage documentation
- See INSTALL.md for installation help
- See EXAMPLES.md for code examples
- Refer to DUCC Python documentation: https://mtr.pages.mpcdf.de/ducc

